/*
 * sdcard.c
 * Copyright (C) 2014 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <assert.h>
#include <xcore/asm.h>
#include <xcore/bits.h>
#include <halm/delay.h>
#include <halm/generic/sdcard.h>
#include <halm/generic/sdio.h>
#include <halm/generic/sdio_defs.h>
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
enum State
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
static enum Result executeCommand(struct SdCard *, uint32_t, uint32_t,
    uint32_t *, bool);
static uint32_t extractBits(const uint32_t *, unsigned int, unsigned int);
static enum Result identifyCard(struct SdCard *);
static enum Result initializeCard(struct SdCard *);
static void interruptHandler(void *);
static enum Result isCardReady(struct SdCard *);
static inline bool isMultiBufferTransfer(struct SdCard *);
static void processCardSpecificData(struct SdCard *, uint32_t *);
static enum Result setTransferState(struct SdCard *);
static enum Result startCardSelection(struct SdCard *);
static enum Result startTransfer(struct SdCard *);
static enum Result startTransferStateSetup(struct SdCard *);
static enum Result terminateTransfer(struct SdCard *);
static enum Result transferBuffer(struct SdCard *, uint32_t, uint32_t,
    uintptr_t, size_t);
/*----------------------------------------------------------------------------*/
static enum Result cardInit(void *, const void *);
static void cardDeinit(void *);
static void cardSetCallback(void *, void (*)(void *), void *);
static enum Result cardGetParam(void *, enum IfParameter, void *);
static enum Result cardSetParam(void *, enum IfParameter, const void *);
static size_t cardRead(void *, void *, size_t);
static size_t cardWrite(void *, const void *, size_t);
/*----------------------------------------------------------------------------*/
const struct InterfaceClass * const SdCard = &(const struct InterfaceClass){
    .size = sizeof(struct SdCard),
    .init = cardInit,
    .deinit = cardDeinit,

    .setCallback = cardSetCallback,
    .getParam = cardGetParam,
    .setParam = cardSetParam,
    .read = cardRead,
    .write = cardWrite
};
/*----------------------------------------------------------------------------*/
static enum Result executeCommand(struct SdCard *device, uint32_t command,
    uint32_t argument, uint32_t *response, bool await)
{
  enum Result res;

  res = ifSetParam(device->interface, IF_SDIO_COMMAND, &command);
  if (res != E_OK)
    return res;
  res = ifSetParam(device->interface, IF_SDIO_ARGUMENT, &argument);
  if (res != E_OK)
    return res;

  enum Result status = ifSetParam(device->interface, IF_SDIO_EXECUTE, 0);

  if (status == E_BUSY)
  {
    if (!await)
      return E_BUSY;

    while ((status = ifGetParam(device->interface, IF_STATUS, 0)) == E_BUSY)
      barrier();
  }

  if (status != E_OK && status != E_IDLE)
    return status;

  if (response)
  {
    res = ifGetParam(device->interface, IF_SDIO_RESPONSE, response);
    if (res != E_OK)
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
static enum Result identifyCard(struct SdCard *device)
{
  uint32_t response[4];
  enum Result res;

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

  const enum SdioResponse responseType = device->mode != SDIO_SPI ?
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
        SDIO_RESPONSE_SHORT, crcStatus), 1UL << BLOCK_POW, 0, true);
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
static enum Result initializeCard(struct SdCard *device)
{
  const uint32_t lowRate = ENUM_RATE;
  uint32_t originalRate;
  enum Result res;

  /* Lock the interface */
  ifSetParam(device->interface, IF_ACQUIRE, 0);

  /* Check interface capabilities and select zero-copy mode */
  res = ifSetParam(device->interface, IF_ZEROCOPY, 0);
  if (res != E_OK)
    goto error;
  ifSetCallback(device->interface, interruptHandler, device);

  res = ifGetParam(device->interface, IF_RATE, &originalRate);
  if (res != E_OK)
    goto error;
  if (originalRate > WORK_RATE)
  {
    res = E_VALUE;
    goto error;
  }

  /* Set low data rate for enumeration purposes */
  res = ifSetParam(device->interface, IF_RATE, &lowRate);
  if (res != E_OK)
    goto error;

  /* Initialize memory card */
  res = identifyCard(device);

  /* Restore original interface rate */
  ifSetParam(device->interface, IF_RATE, &originalRate);

error:
  /* Release the interface */
  ifSetParam(device->interface, IF_RELEASE, 0);

  return res;
}
/*----------------------------------------------------------------------------*/
static void interruptHandler(void *object)
{
  struct SdCard * const device = object;
  bool event = false;

  switch ((enum State)device->state)
  {
    case STATE_GET_STATUS:
    {
      switch (isCardReady(device))
      {
        case E_OK:
          device->state = STATE_TRANSFER;

          if (startTransfer(device) != E_OK)
          {
            event = true;
            device->state = STATE_ERROR;
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
        event = true;
        device->state = STATE_ERROR;
      }
      break;
    }

    case STATE_TRANSFER:
    {
      const enum Result res = ifGetParam(device->interface, IF_STATUS, 0);

      if (res != E_OK)
      {
        device->state = STATE_HALT;
      }
      else
      {
        event = true;
        device->state = STATE_IDLE;
      }

      if (device->state == STATE_HALT)
      {
        if (terminateTransfer(device) != E_OK)
        {
          event = true;
          device->state = STATE_ERROR;
        }
      }
      break;
    }

    case STATE_STOP:
    {
      const enum Result res = ifGetParam(device->interface, IF_STATUS, 0);

      event = true;
      device->state = res == E_OK ? STATE_IDLE : STATE_ERROR;
      break;
    }

    case STATE_HALT:
      event = true;
      device->state = STATE_ERROR;
      break;

    default:
      break;
  }

  if (event)
  {
    ifSetParam(device->interface, IF_RELEASE, 0);

    if (device->callback)
      device->callback(device->callbackArgument);
  }
}
/*----------------------------------------------------------------------------*/
static enum Result isCardReady(struct SdCard *device)
{
  uint32_t response;

  const enum Result res = ifGetParam(device->interface, IF_SDIO_RESPONSE,
      &response);

  if (res != E_OK)
    return res;

  switch ((enum CardState)CURRENT_STATE(response))
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
  const enum SdioCommand code = COMMAND_CODE_VALUE(device->command);

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
static enum Result setTransferState(struct SdCard *device)
{
  /* Relative card address should be initialized */
  const uint32_t address = device->address << 16;
  const uint32_t flags = SDIO_WAIT_DATA | (device->crc ? SDIO_CHECK_CRC : 0);
  uint32_t response;
  enum Result res;

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
static enum Result startCardSelection(struct SdCard *device)
{
  const uint32_t address = device->address << 16;
  const uint32_t flags = SDIO_WAIT_DATA | (device->crc ? SDIO_CHECK_CRC : 0);

  const enum Result res = executeCommand(device, SDIO_COMMAND(CMD_SELECT_CARD,
      SDIO_RESPONSE_SHORT, flags), address, 0, false);

  /* Operation must not be completed instantly */
  return res == E_BUSY ? E_OK : (res == E_OK ? E_INVALID : res);
}
/*----------------------------------------------------------------------------*/
static enum Result startTransfer(struct SdCard *device)
{
  enum Result res;

  res = ifSetParam(device->interface, IF_SDIO_COMMAND, &device->command);
  if (res != E_OK)
    return res;
  res = ifSetParam(device->interface, IF_SDIO_ARGUMENT, &device->argument);
  if (res != E_OK)
    return res;

  const enum SdioCommand code = COMMAND_CODE_VALUE(device->command);
  const size_t length = device->length;
  size_t number;

  if (code == CMD_READ_SINGLE_BLOCK || code == CMD_READ_MULTIPLE_BLOCK)
    number = ifRead(device->interface, (void *)device->buffer, length);
  else
    number = ifWrite(device->interface, (const void *)device->buffer, length);

  return number == length ? E_OK : E_INTERFACE;
}
/*----------------------------------------------------------------------------*/
static enum Result startTransferStateSetup(struct SdCard *device)
{
  const uint32_t address = device->address << 16;
  const uint32_t flags = SDIO_WAIT_DATA | (device->crc ? SDIO_CHECK_CRC : 0);

  const enum Result res = executeCommand(device, SDIO_COMMAND(CMD_SEND_STATUS,
      SDIO_RESPONSE_SHORT, flags), address, 0, false);

  /* Operation must not be completed instantly */
  return res == E_BUSY ? E_OK : (res == E_OK ? E_INVALID : res);
}
/*----------------------------------------------------------------------------*/
static enum Result terminateTransfer(struct SdCard *device)
{
  const uint32_t flags = (device->crc ? SDIO_CHECK_CRC : 0)
      | SDIO_STOP_TRANSFER;
  const uint32_t command = SDIO_COMMAND(CMD_STOP_TRANSMISSION,
      SDIO_RESPONSE_SHORT, flags);

  const enum Result res = executeCommand(device, command, 0, 0, false);

  /* Operation must not be completed instantly */
  return res == E_BUSY ? E_OK : (res == E_OK ? E_INVALID : res);
}
/*----------------------------------------------------------------------------*/
static enum Result transferBuffer(struct SdCard *device,
    uint32_t command, uint32_t argument, uintptr_t buffer, size_t length)
{
  enum Result res;

  ifSetParam(device->interface, IF_ACQUIRE, 0);
  ifSetParam(device->interface, IF_ZEROCOPY, 0);
  ifSetCallback(device->interface, interruptHandler, device);

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
    ifSetParam(device->interface, IF_RELEASE, 0);

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
static enum Result cardInit(void *object, const void *configBase)
{
  const struct SdCardConfig * const config = configBase;
  struct SdCard * const device = object;

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
  const enum Result res = ifGetParam(device->interface, IF_SDIO_MODE,
      &device->mode);

  if (res != E_OK)
    return res;

  return initializeCard(device);
}
/*----------------------------------------------------------------------------*/
static void cardDeinit(void *object)
{
  struct SdCard * const device = object;
  ifSetCallback(device->interface, 0, 0); //FIXME Acquire?
}
/*----------------------------------------------------------------------------*/
static void cardSetCallback(void *object, void (*callback)(void *),
    void *argument)
{
  struct SdCard * const device = object;

  device->callbackArgument = argument;
  device->callback = callback;
}
/*----------------------------------------------------------------------------*/
static enum Result cardGetParam(void *object, enum IfParameter parameter,
    void *data)
{
  struct SdCard * const device = object;

  switch (parameter)
  {
    case IF_POSITION_64:
      *(uint64_t *)data = device->position;
      return E_OK;

    case IF_SIZE_64:
      *(uint64_t *)data = (uint64_t)device->blockCount << BLOCK_POW;
      return E_OK;

    case IF_STATUS:
    {
      switch ((enum State)device->state)
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
static enum Result cardSetParam(void *object, enum IfParameter parameter,
    const void *data)
{
  struct SdCard * const device = object;

  switch (parameter)
  {
    case IF_BLOCKING:
      device->blocking = true;
      return E_OK;

    case IF_POSITION_64:
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

  const enum SdioCommand code = blocks == 1 ?
      CMD_READ_SINGLE_BLOCK : CMD_READ_MULTIPLE_BLOCK;
  const enum SdioResponse response = device->mode != SDIO_SPI ?
      SDIO_RESPONSE_SHORT : SDIO_RESPONSE_NONE;
  const uint32_t command = SDIO_COMMAND(code, response, flags);
  const uint32_t argument = (uint32_t)(device->capacity == SDCARD_SDSC ?
      device->position : (device->position >> BLOCK_POW));

  const enum Result res = transferBuffer(device, command, argument,
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
  const enum SdioCommand code = blocks == 1 ?
      CMD_WRITE_BLOCK : CMD_WRITE_MULTIPLE_BLOCK;
  const enum SdioResponse response = device->mode != SDIO_SPI ?
      SDIO_RESPONSE_SHORT : SDIO_RESPONSE_NONE;
  const uint32_t command = SDIO_COMMAND(code, response, flags);
  const uint32_t argument = (uint32_t)(device->capacity == SDCARD_SDSC ?
      device->position : (device->position >> BLOCK_POW));

  const enum Result res = transferBuffer(device, command, argument,
      (uintptr_t)buffer, length);

  return (res == E_OK || res == E_BUSY) ? length : 0;
}
