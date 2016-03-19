/*
 * sdcard.c
 * Copyright (C) 2014 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <assert.h>
#include <bits.h>
#include <delay.h>
#include <memory.h>
#include <common/sdio.h>
#include <common/sdio_defs.h>
#include <platform/sdcard.h>
/*----------------------------------------------------------------------------*/
#define ENUM_RATE         400000
#define WORK_RATE         25000000
#define BLOCK_POW         9
/*------------------CMD8------------------------------------------------------*/
#define CONDITION_PATTERN 0x000001AA
/*------------------CMD59-----------------------------------------------------*/
#define CRC_ENABLED       0x00000001
/*------------------ACMD6-----------------------------------------------------*/
#define BUS_WIDTH_1BIT    0x00000000
#define BUS_WIDTH_4BIT    0x00000002
/*------------------OCR register----------------------------------------------*/
#define OCR_VOLTAGE_MASK  0x00FF8000
#define OCR_CCS           BIT(30) /* Card Capacity Status */
#define OCR_HCS           BIT(30) /* Host Capacity Support */
#define OCR_BUSY          BIT(31) /* Card power up status bit */
/*----------------------------------------------------------------------------*/
enum state
{
  STATE_IDLE,
  STATE_GET_STATUS,
  STATE_SELECT_CARD,
  STATE_TRANSFER,
  STATE_STOP,
  STATE_HALT,
  STATE_ERROR
};
/*----------------------------------------------------------------------------*/
static enum result executeCommand(struct SdCard *, uint32_t, uint32_t,
    uint32_t *, bool);
static uint32_t extractBits(const uint32_t *, unsigned int, unsigned int);
static enum result identifyCard(struct SdCard *);
static enum result initializeCard(struct SdCard *);
static void interruptHandler(void *);
static enum result isCardReady(struct SdCard *);
static inline bool isMultiBufferTransfer(struct SdCard *);
static void processCardSpecificData(struct SdCard *, uint32_t *);
static enum result setTransferState(struct SdCard *);
static enum result startCardSelection(struct SdCard *);
static enum result startTransfer(struct SdCard *);
static enum result startTransferStateSetup(struct SdCard *);
static enum result terminateTransfer(struct SdCard *);
static enum result transferBuffer(struct SdCard *, uint32_t, uint32_t,
    uintptr_t, size_t);
/*----------------------------------------------------------------------------*/
static enum result cardInit(void *, const void *);
static void cardDeinit(void *);
static enum result cardCallback(void *, void (*)(void *), void *);
static enum result cardGet(void *, enum ifOption, void *);
static enum result cardSet(void *, enum ifOption, const void *);
static size_t cardRead(void *, void *, size_t);
static size_t cardWrite(void *, const void *, size_t);
/*----------------------------------------------------------------------------*/
static const struct InterfaceClass cardTable = {
    .size = sizeof(struct SdCard),
    .init = cardInit,
    .deinit = cardDeinit,

    .callback = cardCallback,
    .get = cardGet,
    .set = cardSet,
    .read = cardRead,
    .write = cardWrite
};
/*----------------------------------------------------------------------------*/
const struct InterfaceClass * const SdCard = &cardTable;
/*----------------------------------------------------------------------------*/
static enum result executeCommand(struct SdCard *device, uint32_t command,
    uint32_t argument, uint32_t *response, bool await)
{
  enum result res;

  if ((res = ifSet(device->interface, IF_SDIO_COMMAND, &command)) != E_OK)
    return res;
  if ((res = ifSet(device->interface, IF_SDIO_ARGUMENT, &argument)) != E_OK)
    return res;

  enum result status = ifSet(device->interface, IF_SDIO_EXECUTE, 0);

  if (status == E_BUSY)
  {
    if (!await)
      return E_BUSY;

    while ((status = ifGet(device->interface, IF_STATUS, 0)) == E_BUSY)
      barrier();
  }

  if (status != E_OK && status != E_IDLE)
    return status;

  if (response)
  {
    if ((res = ifGet(device->interface, IF_SDIO_RESPONSE, response)) != E_OK)
      return res;
  }

  return status;
}
/*----------------------------------------------------------------------------*/
static uint32_t extractBits(const uint32_t *data, unsigned int start,
    unsigned int end)
{
  const unsigned int index = end >> 5;
  const unsigned int offset = start & 0x1F;
  uint32_t value;

  if (index == start >> 5)
    value = data[index] >> offset;
  else
    value = (data[index] << (32 - offset)) | (data[start >> 5] >> offset);

  return value & MASK(end - start + 1);
}
/*----------------------------------------------------------------------------*/
static enum result identifyCard(struct SdCard *device)
{
  uint32_t response[4];
  enum result res;

  /* Send reset command */
  res = executeCommand(device, SDIO_COMMAND(CMD_GO_IDLE_STATE,
      SDIO_RESPONSE_NONE, SDIO_INITIALIZE), 0, 0, true);
  if (res != E_OK && res != E_IDLE)
    return res;

  /* Start initialization and detect card type */
  res = executeCommand(device, SDIO_COMMAND(CMD_SEND_IF_COND,
      SDIO_RESPONSE_SHORT, 0), CONDITION_PATTERN, response, true);
  if (res == E_OK || res == E_IDLE)
  {
    /* Response should be equal to the command argument */
    if (response[0] != CONDITION_PATTERN)
      return E_DEVICE;

    device->type = SDCARD_2_0;
  }
  else if (res != E_INVALID && res != E_TIMEOUT)
  {
    /* Other error, it is not an unsupported command */
    return res;
  }

  const enum sdioResponse responseType = device->mode != SDIO_SPI ?
      SDIO_RESPONSE_SHORT : SDIO_RESPONSE_NONE;
  uint32_t ocr = 0;

  if (device->type == SDCARD_2_0)
    ocr |= OCR_HCS;
  if (device->mode != SDIO_SPI)
    ocr |= OCR_VOLTAGE_MASK;

  /* Wait till card becomes ready */
  for (unsigned int counter = 100; counter; --counter)
  {
    res = executeCommand(device, SDIO_COMMAND(CMD_APP_CMD,
        responseType, 0), 0, 0, true);
    if (res != E_OK && res != E_IDLE)
      break;

    res = executeCommand(device, SDIO_COMMAND(ACMD_SD_SEND_OP_COND,
        responseType, 0), ocr, device->mode != SDIO_SPI ? response : 0, true);
    if (device->mode != SDIO_SPI)
    {
      if (res == E_OK && (response[0] & OCR_BUSY))
      {
        /* Check card capacity information */
        if (response[0] & OCR_CCS)
          device->capacity = SDCARD_SDHC;
        break;
      }
    }
    else
    {
      if (res != E_IDLE)
        break;
    }

    /* TODO Remove delay */
    mdelay(10);
  }
  if (res != E_OK)
    return res;

  const uint32_t crcStatus = device->crc ? SDIO_CHECK_CRC : 0;

  /* Enable optional integrity checking */
  if (device->mode == SDIO_SPI && device->crc)
  {
    res = executeCommand(device, SDIO_COMMAND(CMD_CRC_ON_OFF,
        SDIO_RESPONSE_SHORT, 0), CRC_ENABLED, 0, true);
    if (res != E_OK)
      return res;
  }

  /* Read card capacity information when SPI mode is used */
  if (device->mode == SDIO_SPI && device->type == SDCARD_2_0)
  {
    res = executeCommand(device, SDIO_COMMAND(CMD_READ_OCR,
        SDIO_RESPONSE_SHORT, crcStatus), 0, response, true);
    if (res != E_OK)
      return res;

    if (response[0] & OCR_CCS)
      device->capacity = SDCARD_SDHC;
  }

  /* Read CID and RCA information */
  if (device->mode != SDIO_SPI)
  {
    res = executeCommand(device, SDIO_COMMAND(CMD_ALL_SEND_CID,
        SDIO_RESPONSE_LONG, crcStatus), 0, response, true);
    if (res != E_OK)
      return res;

    res = executeCommand(device, SDIO_COMMAND(CMD_SEND_RELATIVE_ADDR,
        SDIO_RESPONSE_SHORT, crcStatus), 0, response, true);
    if (res != E_OK)
      return res;

    device->address = response[0] >> 16;
  }

  const uint32_t address = device->address << 16;

  /* Read and process CSD register */
  res = executeCommand(device, SDIO_COMMAND(CMD_SEND_CSD,
      SDIO_RESPONSE_LONG, crcStatus), address, response, true);
  if (res != E_OK)
    return res;
  processCardSpecificData(device, response);

  /* Configure block length and bus width */
  if (device->mode != SDIO_SPI)
  {
    if ((res = setTransferState(device)) != E_OK)
      return res;

    res = executeCommand(device, SDIO_COMMAND(CMD_SET_BLOCKLEN,
        SDIO_RESPONSE_SHORT, crcStatus), 1 << BLOCK_POW, 0, true);
    if (res != E_OK)
      return res;

    res = executeCommand(device, SDIO_COMMAND(CMD_APP_CMD,
        SDIO_RESPONSE_SHORT, crcStatus), address, 0, true);
    if (res != E_OK)
      return res;

    const uint8_t width = device->mode == SDIO_1BIT ?
        BUS_WIDTH_1BIT : BUS_WIDTH_4BIT;

    res = executeCommand(device, SDIO_COMMAND(ACMD_SET_BUS_WIDTH,
        SDIO_RESPONSE_SHORT, crcStatus), width, 0, true);
    if (res != E_OK)
      return res;
  }

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static enum result initializeCard(struct SdCard *device)
{
  const uint32_t lowRate = ENUM_RATE;
  uint32_t originalRate;
  enum result res;

  /* Lock the interface */
  ifSet(device->interface, IF_ACQUIRE, 0);

  /* Check interface capabilities and select zero-copy mode */
  if ((res = ifSet(device->interface, IF_ZEROCOPY, 0)) != E_OK)
    goto error;
  if ((res = ifCallback(device->interface, interruptHandler, device)) != E_OK)
    goto error;

  if ((res = ifGet(device->interface, IF_RATE, &originalRate)) != E_OK)
    goto error;
  if (originalRate > WORK_RATE)
  {
    res = E_VALUE;
    goto error;
  }

  /* Set low data rate for enumeration purposes */
  if ((res = ifSet(device->interface, IF_RATE, &lowRate)) != E_OK)
    goto error;

  /* Initialize memory card */
  res = identifyCard(device);

  /* Restore original interface rate */
  ifSet(device->interface, IF_RATE, &originalRate);

error:
  /* Release the interface */
  ifSet(device->interface, IF_RELEASE, 0);

  return res;
}
/*----------------------------------------------------------------------------*/
static void interruptHandler(void *object)
{
  struct SdCard * const device = object;
  bool event = false;

  switch ((enum state)device->state)
  {
    case STATE_GET_STATUS:
    {
      switch (isCardReady(device))
      {
        case E_OK:
          device->state = STATE_TRANSFER;

          if (startTransfer(device) != E_OK)
          {
            device->state = STATE_ERROR;
            event = true;
          }
          break;

        case E_IDLE:
          device->state = STATE_SELECT_CARD;

          if (startCardSelection(device) != E_OK)
          {
            event = true;
            device->state = STATE_ERROR;
          }
          break;

        default:
          event = true;
          device->state = STATE_ERROR;
          break;
      }

      break;
    }

    case STATE_SELECT_CARD:
    {
      device->state = STATE_TRANSFER;

      if (startTransfer(device) != E_OK)
      {
        device->state = STATE_ERROR;
        event = true;
      }
      break;
    }

    case STATE_TRANSFER:
    {
      const enum result res = ifGet(device->interface, IF_STATUS, 0);

      if (res != E_OK)
      {
        device->state = STATE_HALT;
      }
      else if (isMultiBufferTransfer(device))
      {
        device->state = STATE_STOP;
      }
      else
      {
        device->state = STATE_IDLE;
        event = true;
      }

      if (device->state == STATE_HALT || device->state == STATE_STOP)
      {
        if (terminateTransfer(device) != E_OK)
        {
          device->state = STATE_ERROR;
          event = true;
        }
      }
      break;
    }

    case STATE_STOP:
      device->state = STATE_IDLE;
      event = true;
      break;

    case STATE_HALT:
      device->state = STATE_ERROR;
      event = true;

    default:
      break;
  }

  if (event)
  {
    ifSet(device->interface, IF_RELEASE, 0);

    if (device->callback)
      device->callback(device->callbackArgument);
  }
}
/*----------------------------------------------------------------------------*/
static enum result isCardReady(struct SdCard *device)
{
  uint32_t response;
  enum result res;

  if ((res = ifGet(device->interface, IF_SDIO_RESPONSE, &response)) != E_OK)
    return res;

  const uint8_t state = CURRENT_STATE(response);

  switch ((enum cardState)state)
  {
    case CARD_TRANSFER:
      return E_OK;

    case CARD_STANDBY:
      return E_IDLE;

    default:
      return E_BUSY;
  }
}
/*----------------------------------------------------------------------------*/
static inline bool isMultiBufferTransfer(struct SdCard *device)
{
  const enum sdioCommand code = COMMAND_CODE_VALUE(device->command);

  return code == CMD_READ_MULTIPLE_BLOCK || code == CMD_WRITE_MULTIPLE_BLOCK;
}
/*----------------------------------------------------------------------------*/
static void processCardSpecificData(struct SdCard *device, uint32_t *response)
{
  if (device->capacity == SDCARD_SDSC)
  {
    const uint32_t blockLength = extractBits(response, 80, 83);
    const uint32_t deviceSize = extractBits(response, 62, 73) + 1;
    const uint32_t sizeMultiplier = extractBits(response, 47, 49);

    device->blockCount = (deviceSize << (sizeMultiplier + 2));
    device->blockCount <<= blockLength - 9;
  }
  else
  {
    const uint32_t deviceSize = extractBits(response, 48, 69) + 1;

    device->blockCount = deviceSize << 10;
  }
}
/*----------------------------------------------------------------------------*/
static enum result setTransferState(struct SdCard *device)
{
  /* Relative card address should be initialized */
  const uint32_t address = device->address << 16;
  const uint32_t flags = SDIO_WAIT_DATA | (device->crc ? SDIO_CHECK_CRC : 0);
  uint32_t response;
  enum result res;

  res = executeCommand(device, SDIO_COMMAND(CMD_SEND_STATUS,
      SDIO_RESPONSE_SHORT, flags), address, &response, true);
  if (res != E_OK)
    return res;

  const uint8_t state = CURRENT_STATE(response);

  if (state == CARD_STANDBY)
  {
    return executeCommand(device, SDIO_COMMAND(CMD_SELECT_CARD,
        SDIO_RESPONSE_SHORT, flags), address, 0, true);
  }
  else if (state != CARD_TRANSFER)
  {
    return res;
  }
  else
    return E_OK;
}
/*----------------------------------------------------------------------------*/
static enum result startCardSelection(struct SdCard *device)
{
  const uint32_t address = device->address << 16;
  const uint32_t flags = SDIO_WAIT_DATA | (device->crc ? SDIO_CHECK_CRC : 0);

  const enum result res = executeCommand(device, SDIO_COMMAND(CMD_SEND_STATUS,
      SDIO_RESPONSE_SHORT, flags), address, 0, false);

  /* Operation must not be completed instantly */
  return res == E_BUSY ? E_OK : (res == E_OK ? E_INVALID : res);
}
/*----------------------------------------------------------------------------*/
static enum result startTransfer(struct SdCard *device)
{
  enum result res;

  res = ifSet(device->interface, IF_SDIO_COMMAND, &device->command);
  if (res != E_OK)
    return res;
  res = ifSet(device->interface, IF_SDIO_ARGUMENT, &device->argument);
  if (res != E_OK)
    return res;

  const enum sdioCommand code = COMMAND_CODE_VALUE(device->command);
  const size_t length = device->length;
  size_t number;

  if (code == CMD_READ_SINGLE_BLOCK || code == CMD_READ_MULTIPLE_BLOCK)
  {
    number = ifRead(device->interface, (void *)device->buffer, length);
  }
  else
  {
    number = ifWrite(device->interface, (const void *)device->buffer,
        length);
  }

  return number == length ? E_OK : E_INTERFACE;
}
/*----------------------------------------------------------------------------*/
static enum result startTransferStateSetup(struct SdCard *device)
{
  const uint32_t address = device->address << 16;
  const uint32_t flags = SDIO_WAIT_DATA | (device->crc ? SDIO_CHECK_CRC : 0);

  const enum result res = executeCommand(device, SDIO_COMMAND(CMD_SEND_STATUS,
      SDIO_RESPONSE_SHORT, flags), address, 0, false);

  /* Operation must not be completed instantly */
  return res == E_BUSY ? E_OK : (res == E_OK ? E_INVALID : res);
}
/*----------------------------------------------------------------------------*/
static enum result terminateTransfer(struct SdCard *device)
{
  const uint32_t flags = (device->crc ? SDIO_CHECK_CRC : 0)
      | SDIO_STOP_TRANSFER;
  const uint32_t command = SDIO_COMMAND(CMD_STOP_TRANSMISSION,
      SDIO_RESPONSE_SHORT, flags);

  const enum result res = executeCommand(device, command, 0, 0, false);

  /* Operation must not be completed instantly */
  return res == E_BUSY ? E_OK : (res == E_OK ? E_INVALID : res);
}
/*----------------------------------------------------------------------------*/
static enum result transferBuffer(struct SdCard *device,
    uint32_t command, uint32_t argument, uintptr_t buffer, size_t length)
{
  enum result res;

  ifSet(device->interface, IF_ACQUIRE, 0);
  ifSet(device->interface, IF_ZEROCOPY, 0);
  ifCallback(device->interface, interruptHandler, device);

  device->argument = argument;
  device->command = command;
  device->buffer = buffer;
  device->length = length;

  if (device->mode != SDIO_SPI)
  {
    device->state = STATE_GET_STATUS;
    res = startTransferStateSetup(device);
  }
  else
  {
    device->state = STATE_TRANSFER;
    res = startTransfer(device);
  }

  if (res != E_OK)
  {
    device->state = STATE_ERROR;
    ifSet(device->interface, IF_RELEASE, 0);

    if (device->callback)
      device->callback(device->callbackArgument);

    return res;
  }

  if (device->blocking)
  {
    while (device->state != STATE_IDLE && device->state != STATE_ERROR)
      barrier();

    if (device->state == STATE_ERROR)
      res = E_INTERFACE;
    else
      res = E_OK;
  }
  else
    res = E_BUSY;

  return res;
}
/*----------------------------------------------------------------------------*/
static enum result cardInit(void *object, const void *configBase)
{
  const struct SdCardConfig * const config = configBase;
  struct SdCard * const device = object;
  enum result res;

  device->callback = 0;

  device->interface = config->interface;
  device->position = 0;

  device->blockCount = 0;
  device->address = 0;
  device->capacity = SDCARD_SDSC;
  device->type = SDCARD_1_0;
  device->crc = config->crc;

  device->state = STATE_IDLE;
  device->blocking = true;

  /* Get interface type */
  if ((res = ifGet(device->interface, IF_SDIO_MODE, &device->mode)) != E_OK)
    return res;

  return initializeCard(device);
}
/*----------------------------------------------------------------------------*/
static void cardDeinit(void *object)
{
  struct SdCard * const device = object;

  ifCallback(device->interface, 0, 0); //FIXME Acquire?
}
/*----------------------------------------------------------------------------*/
static enum result cardCallback(void *object, void (*callback)(void *),
    void *argument)
{
  struct SdCard * const device = object;

  device->callbackArgument = argument;
  device->callback = callback;
  return E_OK;
}
/*----------------------------------------------------------------------------*/
static enum result cardGet(void *object, enum ifOption option, void *data)
{
  struct SdCard * const device = object;

  switch (option)
  {
    case IF_POSITION:
      *(uint64_t *)data = device->position;
      return E_OK;

    case IF_SIZE:
      *(uint64_t *)data = (uint64_t)device->blockCount << BLOCK_POW;
      return E_OK;

    case IF_STATUS:
    {
      switch ((enum state)device->state)
      {
        case STATE_IDLE:
          return E_OK;

        case STATE_ERROR:
          return E_ERROR;

        default:
          return E_BUSY;
      }
    }

    default:
      return E_INVALID;
  }
}
/*----------------------------------------------------------------------------*/
static enum result cardSet(void *object, enum ifOption option,
    const void *data)
{
  struct SdCard * const device = object;

  switch (option)
  {
    case IF_BLOCKING:
      device->blocking = true;
      return E_OK;

    case IF_POSITION:
    {
      const uint64_t position = *(const uint64_t *)data;

      /* Check address alignment */
      if (position & MASK(BLOCK_POW))
        return E_VALUE;

      /* Check address range */
      if ((position >> BLOCK_POW) >= (uint64_t)device->blockCount)
        return E_VALUE;

      device->position = position;
      return E_OK;
    }

    case IF_ZEROCOPY:
      device->blocking = false;
      return E_OK;

    default:
      return E_INVALID;
  }
}
/*----------------------------------------------------------------------------*/
static size_t cardRead(void *object, void *buffer, size_t length)
{
  assert((length & MASK(BLOCK_POW)) == 0);

  const uint32_t blocks = length >> BLOCK_POW;
  struct SdCard * const device = object;

  if (!blocks)
    return 0;

  uint32_t flags = SDIO_DATA_MODE;

  if (blocks > 1)
    flags |= SDIO_AUTO_STOP;
  if (device->crc)
    flags |= SDIO_CHECK_CRC;

  const enum sdioCommand code = blocks == 1 ?
      CMD_READ_SINGLE_BLOCK : CMD_READ_MULTIPLE_BLOCK;
  const enum sdioResponse response = device->mode != SDIO_SPI ?
      SDIO_RESPONSE_SHORT : SDIO_RESPONSE_NONE;
  const uint32_t command = SDIO_COMMAND(code, response, flags);
  const uint32_t argument = (uint32_t)(device->capacity == SDCARD_SDSC ?
      device->position : (device->position >> BLOCK_POW));

  const enum result res = transferBuffer(device, command, argument,
      (uintptr_t)buffer, length);

  return (res == E_OK || res == E_BUSY) ? length : 0;
}
/*----------------------------------------------------------------------------*/
static size_t cardWrite(void *object, const void *buffer, size_t length)
{
  assert((length & MASK(BLOCK_POW)) == 0);

  const uint32_t blocks = length >> BLOCK_POW;
  struct SdCard * const device = object;

  if (!blocks)
    return 0;

  uint32_t flags = SDIO_DATA_MODE | SDIO_WRITE_MODE;

  if (blocks > 1)
    flags |= SDIO_AUTO_STOP;
  if (device->crc)
    flags |= SDIO_CHECK_CRC;

  /* TODO Protect position reading */
  const enum sdioCommand code = blocks == 1 ?
      CMD_WRITE_BLOCK : CMD_WRITE_MULTIPLE_BLOCK;
  const enum sdioResponse response = device->mode != SDIO_SPI ?
      SDIO_RESPONSE_SHORT : SDIO_RESPONSE_NONE;
  const uint32_t command = SDIO_COMMAND(code, response, flags);
  const uint32_t argument = (uint32_t)(device->capacity == SDCARD_SDSC ?
      device->position : (device->position >> BLOCK_POW));

  const enum result res = transferBuffer(device, command, argument,
      (uintptr_t)buffer, length);

  return (res == E_OK || res == E_BUSY) ? length : 0;
}
