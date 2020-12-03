/*
 * mmcsd.c
 * Copyright (C) 2014, 2020 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/delay.h>
#include <halm/generic/mmcsd.h>
#include <halm/generic/mmcsd_defs.h>
#include <halm/generic/sdio.h>
#include <halm/generic/sdio_defs.h>
#include <xcore/asm.h>
#include <xcore/memory.h>
#include <xcore/bits.h>
#include <assert.h>
#include <string.h>
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
static enum Result initStepEnableCrc(struct MMCSD *);
static enum Result initStepMmcReadOCR(struct MMCSD *);
static enum Result initStepMmcSetBusWidth(struct MMCSD *);
static enum Result initStepReadCondition(struct MMCSD *);
static enum Result initStepReadCID(struct MMCSD *);
static enum Result initStepReadCSD(struct MMCSD *);
static enum Result initStepReadExtCSD(struct MMCSD *);
static enum Result initStepReadRCA(struct MMCSD *);
static enum Result initStepSdReadOCR(struct MMCSD *, enum MMCSDResponse);
static enum Result initStepSdSetBusWidth(struct MMCSD *);
static enum Result initStepSendReset(struct MMCSD *);
static enum Result initStepSetBlockLength(struct MMCSD *);
static enum Result initStepSetRCA(struct MMCSD *, uint32_t);
static enum Result initStepSpiReadOCR(struct MMCSD *);
/*----------------------------------------------------------------------------*/
static enum Result executeCommand(struct MMCSD *, uint32_t, uint32_t,
    uint32_t *, bool);
static uint32_t extractBits(const uint32_t *, unsigned int, unsigned int);
static enum Result identifyCard(struct MMCSD *);
static enum Result initializeCard(struct MMCSD *);
static void interruptHandler(void *);
static enum Result isCardReady(struct MMCSD *);
static inline bool isMultiBufferTransfer(struct MMCSD *);
static void parseCardSpecificData(struct MMCSD *, uint32_t *);
static enum Result setTransferState(struct MMCSD *);
static enum Result startCardSelection(struct MMCSD *);
static enum Result startTransfer(struct MMCSD *);
static enum Result startTransferStateSetup(struct MMCSD *);
static enum Result terminateTransfer(struct MMCSD *);
static enum Result transferBuffer(struct MMCSD *, uint32_t, uint32_t,
    uintptr_t, size_t);
/*----------------------------------------------------------------------------*/
static enum Result cardInit(void *, const void *);
static void cardDeinit(void *);
static void cardSetCallback(void *, void (*)(void *), void *);
static enum Result cardGetParam(void *, int, void *);
static enum Result cardSetParam(void *, int, const void *);
static size_t cardRead(void *, void *, size_t);
static size_t cardWrite(void *, const void *, size_t);
/*----------------------------------------------------------------------------*/
const struct InterfaceClass * const MMCSD = &(const struct InterfaceClass){
    .size = sizeof(struct MMCSD),
    .init = cardInit,
    .deinit = cardDeinit,

    .setCallback = cardSetCallback,
    .getParam = cardGetParam,
    .setParam = cardSetParam,
    .read = cardRead,
    .write = cardWrite
};

/*----------------------------------------------------------------------------*/
static enum Result initStepEnableCrc(struct MMCSD *device)
{
  return executeCommand(device,
      SDIO_COMMAND(CMD59_CRC_ON_OFF, MMCSD_RESPONSE_R1, 0),
      CRC_ENABLED, 0, true);
}
/*----------------------------------------------------------------------------*/
static enum Result initStepMmcReadOCR(struct MMCSD *device)
{
  enum Result res;

  for (unsigned int counter = 100; counter; --counter)
  {
    uint32_t response;

    res = executeCommand(device,
        SDIO_COMMAND(CMD1_SEND_OP_COND, MMCSD_RESPONSE_R3, 0),
        OCR_VOLTAGE_MASK, &response, true);

    if (res == E_OK)
    {
      /* Status bit is active low */
      if (response & OCR_BUSY)
      {
        if (response & OCR_MMC_SECTOR_MODE)
        {
          /* Card capacity is greater than 2GB */
          device->capacity = MMCSD_HC;
        }
        else
        {
          /* Card capacity is less then or equal to 2GB */
          device->capacity = MMCSD_SC;
        }

        device->type = CARD_MMC;
        break;
      }
    }
    else
      break;

    /* TODO Remove delay */
    mdelay(10);
  }

  return res;
}
/*----------------------------------------------------------------------------*/
static enum Result initStepMmcSetBusWidth(struct MMCSD *device)
{
  uint32_t busMode = 0x03B70000UL;

  if (device->mode == SDIO_8BIT)
    busMode |= MMC_BUS_WIDTH_8BIT;
  else if (device->mode == SDIO_4BIT)
    busMode |= MMC_BUS_WIDTH_4BIT;

  return executeCommand(device,
      SDIO_COMMAND(CMD6_SWITCH, MMCSD_RESPONSE_R1B, SDIO_CHECK_CRC),
      busMode, 0, true);
}
/*----------------------------------------------------------------------------*/
static enum Result initStepReadCondition(struct MMCSD *device)
{
  uint32_t response;
  const enum Result res = executeCommand(device,
      SDIO_COMMAND(CMD8_SEND_IF_COND, MMCSD_RESPONSE_R7, 0),
      CONDITION_PATTERN, &response, true);

  if (res == E_OK || res == E_IDLE)
  {
    /* Response should be equal to the command argument */
    if (response != CONDITION_PATTERN)
      return E_DEVICE;

    device->type = CARD_SD_2_0;
  }
  else if (res != E_INVALID && res != E_TIMEOUT)
  {
    /* Other error, it's not an unsupported command */
    return res;
  }

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static enum Result initStepReadCID(struct MMCSD *device)
{
  return executeCommand(device,
      SDIO_COMMAND(CMD2_ALL_SEND_CID, MMCSD_RESPONSE_R2, SDIO_CHECK_CRC),
      0, 0, true);
}
/*----------------------------------------------------------------------------*/
static enum Result initStepReadCSD(struct MMCSD *device)
{
  uint32_t response[4];
  const enum Result res = executeCommand(device,
      SDIO_COMMAND(CMD9_SEND_CSD, MMCSD_RESPONSE_R2, SDIO_CHECK_CRC),
      (device->address << 16), response, true);

  if (res == E_OK)
    parseCardSpecificData(device, response);
  return res;
}
/*----------------------------------------------------------------------------*/
static enum Result initStepReadExtCSD(struct MMCSD *device)
{
  static const uint32_t argument = 0;
  static const uint32_t command = SDIO_COMMAND(CMD8_SEND_EXT_CSD,
      MMCSD_RESPONSE_R1, SDIO_DATA_MODE | SDIO_CHECK_CRC);

  uint8_t csd[512];
  enum Result res;

  res = ifSetParam(device->interface, IF_SDIO_COMMAND, &command);
  if (res != E_OK)
    return res;
  res = ifSetParam(device->interface, IF_SDIO_ARGUMENT, &argument);
  if (res != E_OK)
    return res;

  const size_t queued = ifRead(device->interface, csd, sizeof(csd));
  enum Result status;

  if (queued == sizeof(csd))
  {
    while ((status = ifGetParam(device->interface, IF_STATUS, 0)) == E_BUSY)
      barrier();
  }
  else
    return E_INTERFACE;

  /* Process SEC_COUNT parameter [215:212] */
  uint32_t sectors;
  memcpy(&sectors, &csd[212], sizeof(sectors));
  device->sectors = fromLittleEndian32(sectors);

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static enum Result initStepReadRCA(struct MMCSD *device)
{
  uint32_t response;
  const enum Result res = executeCommand(device,
      SDIO_COMMAND(CMD3_SEND_RELATIVE_ADDR, MMCSD_RESPONSE_R6, SDIO_CHECK_CRC),
      0, &response, true);

  if (res == E_OK)
    device->address = response >> 16;
  return res;
}
/*----------------------------------------------------------------------------*/
static enum Result initStepSdReadOCR(struct MMCSD *device,
    enum MMCSDResponse responseType)
{
  unsigned int counter = 100;
  uint32_t ocr = 0;
  uint32_t response;
  enum Result res;

  if (device->type == CARD_SD_2_0)
    ocr |= OCR_SD_HCS;
  if (device->mode != SDIO_SPI)
    ocr |= OCR_VOLTAGE_MASK;

  for (; counter; --counter)
  {
    res = executeCommand(device,
        SDIO_COMMAND(CMD55_APP_CMD, responseType, 0),
        0, 0, true);
    if (res != E_OK && res != E_IDLE)
      break;

    res = executeCommand(device,
        SDIO_COMMAND(ACMD41_SD_SEND_OP_COND, responseType, 0),
        ocr, responseType == MMCSD_RESPONSE_R1 ? &response : 0, true);

    if (responseType == MMCSD_RESPONSE_R1)
    {
      if (res == E_OK && (response & OCR_BUSY))
      {
        /* Check card capacity information */
        if (response & OCR_SD_CCS)
          device->capacity = MMCSD_HC;
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

  if (!counter)
    res = E_TIMEOUT;

  return res;
}
/*----------------------------------------------------------------------------*/
static enum Result initStepSdSetBusWidth(struct MMCSD *device)
{
  enum Result res;

  res = executeCommand(device,
      SDIO_COMMAND(CMD55_APP_CMD, MMCSD_RESPONSE_R1, SDIO_CHECK_CRC),
      (device->address << 16), 0, true);
  if (res != E_OK)
    return res;

  const uint8_t busMode = device->mode == SDIO_4BIT ? SD_BUS_WIDTH_4BIT : 0;

  return executeCommand(device,
      SDIO_COMMAND(ACMD6_SET_BUS_WIDTH, MMCSD_RESPONSE_R1, SDIO_CHECK_CRC),
      busMode, 0, true);
}
/*----------------------------------------------------------------------------*/
static enum Result initStepSendReset(struct MMCSD *device)
{
  const enum Result res = executeCommand(device,
      SDIO_COMMAND(CMD0_GO_IDLE_STATE, MMCSD_RESPONSE_NONE, SDIO_INITIALIZE),
      0, 0, true);

  return (res != E_OK && res != E_IDLE) ? res : E_OK;
}
/*----------------------------------------------------------------------------*/
static enum Result initStepSetBlockLength(struct MMCSD *device)
{
  return executeCommand(device,
      SDIO_COMMAND(CMD16_SET_BLOCKLEN, MMCSD_RESPONSE_R1, SDIO_CHECK_CRC),
      1UL << BLOCK_POW, 0, true);
}
/*----------------------------------------------------------------------------*/
static enum Result initStepSetRCA(struct MMCSD *device, uint32_t address)
{
  const enum Result res = executeCommand(device,
      SDIO_COMMAND(CMD3_SEND_RELATIVE_ADDR, MMCSD_RESPONSE_R1, SDIO_CHECK_CRC),
      address << 16, 0, true);

  if (res == E_OK)
    device->address = address;
  return res;
}
/*----------------------------------------------------------------------------*/
static enum Result initStepSpiReadOCR(struct MMCSD *device)
{
  uint32_t response;
  const enum Result res = executeCommand(device,
      SDIO_COMMAND(CMD58_READ_OCR, MMCSD_RESPONSE_R1, SDIO_CHECK_CRC),
      0, &response, true);

  if (res == E_OK && (response & OCR_SD_CCS))
    device->capacity = MMCSD_HC;
  return res;
}
/*----------------------------------------------------------------------------*/
static enum Result executeCommand(struct MMCSD *device, uint32_t command,
    uint32_t argument, uint32_t *response, bool await)
{
  enum Result res;

  if ((command & COMMAND_FLAG(SDIO_CHECK_CRC)) && !device->crc)
    command &= ~COMMAND_FLAG(SDIO_CHECK_CRC);

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
static enum Result identifyCard(struct MMCSD *device)
{
  enum Result res;

  /* Send Reset command */
  if ((res = initStepSendReset(device)) != E_OK)
    return res;

  /* Start initialization and detect card type */
  if ((res = initStepReadCondition(device)) != E_OK)
    return res;

  res = initStepSdReadOCR(device, device->mode == SDIO_SPI ?
      MMCSD_RESPONSE_NONE : MMCSD_RESPONSE_R1);
  if (res == E_TIMEOUT)
  {
    /*
     * Initialization using CMD55 and ACMD42 failed.
     * Try to initialize as MMC card.
     */
    res = initStepMmcReadOCR(device);
  }
  if (res != E_OK)
    return res;

  /* Enable optional integrity checking */
  if (device->mode == SDIO_SPI && device->crc)
  {
    if ((res = initStepEnableCrc(device)) != E_OK)
      return res;
  }

  /* Read card capacity information when SPI mode is used */
  if (device->mode == SDIO_SPI && device->type == CARD_SD_2_0)
  {
    if ((res = initStepSpiReadOCR(device)) != E_OK)
      return res;
  }

  /* Read CID and RCA information */
  if (device->mode != SDIO_SPI)
  {
    if ((res = initStepReadCID(device)) != E_OK)
      return res;

    if (device->type == CARD_MMC)
      res = initStepSetRCA(device, 1);
    else
      res = initStepReadRCA(device);

    if (res != E_OK)
      return res;
  }

  /* Read and process CSD register */
  if ((res = initStepReadCSD(device)) != E_OK)
    return res;

  /* Configure block length and bus width */
  if (device->mode != SDIO_SPI)
  {
    if ((res = setTransferState(device)) != E_OK)
      return res;
    if ((res = initStepSetBlockLength(device)) != E_OK)
      return res;

    if (device->type == CARD_MMC)
    {
      if ((res = initStepMmcSetBusWidth(device)) != E_OK)
        return res;
    }
    else
    {
      if ((res = initStepSdSetBusWidth(device)) != E_OK)
        return res;
    }
  }

  /* Read Extended CSD register of the MMC */
  if (device->type == CARD_MMC && device->capacity == MMCSD_HC)
  {
    if ((res = initStepReadExtCSD(device)) != E_OK)
      return res;
  }

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static enum Result initializeCard(struct MMCSD *device)
{
  static const uint32_t enumRate = ENUM_RATE;
  uint32_t workRate;
  enum Result res;

  /* Get the type of the interface */
  res = ifGetParam(device->interface, IF_SDIO_MODE, &device->mode);
  if (res != E_OK)
    return res;

  /* Lock the interface */
  ifSetParam(device->interface, IF_ACQUIRE, 0);

  /* Check interface capabilities and select zero-copy mode */
  res = ifSetParam(device->interface, IF_ZEROCOPY, 0);
  if (res != E_OK)
    goto error;
  ifSetCallback(device->interface, interruptHandler, device);

  res = ifGetParam(device->interface, IF_RATE, &workRate);
  if (res != E_OK)
    goto error;

  /* Set low data rate for enumeration purposes */
  res = ifSetParam(device->interface, IF_RATE, &enumRate);
  if (res != E_OK)
    goto error;

  /* Initialize memory card */
  res = identifyCard(device);

  /* Restore original interface rate */
  ifSetParam(device->interface, IF_RATE, &workRate);

error:
  /* Release the interface */
  ifSetParam(device->interface, IF_RELEASE, 0);

  return res;
}
/*----------------------------------------------------------------------------*/
static void interruptHandler(void *object)
{
  struct MMCSD * const device = object;
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
static enum Result isCardReady(struct MMCSD *device)
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
static inline bool isMultiBufferTransfer(struct MMCSD *device)
{
  const enum MMCSDCommand code = COMMAND_CODE_VALUE(device->command);

  return code == CMD18_READ_MULTIPLE_BLOCK
      || code == CMD25_WRITE_MULTIPLE_BLOCK;
}
/*----------------------------------------------------------------------------*/
static void parseCardSpecificData(struct MMCSD *device, uint32_t *response)
{
  if (device->capacity == MMCSD_SC)
  {
    const uint32_t blockLength = extractBits(response, 80, 83);
    const uint32_t deviceSize = extractBits(response, 62, 73) + 1;
    const uint32_t sizeMultiplier = extractBits(response, 47, 49);

    device->sectors = (deviceSize << (sizeMultiplier + 2));
    device->sectors <<= blockLength - 9;
  }
  else if (device->type != CARD_MMC)
  {
    const uint32_t deviceSize = extractBits(response, 48, 69) + 1;

    device->sectors = deviceSize << 10;
  }
}
/*----------------------------------------------------------------------------*/
static enum Result setTransferState(struct MMCSD *device)
{
  /* Relative card address should be initialized */
  const uint32_t address = device->address << 16;
  const uint32_t flags = SDIO_WAIT_DATA | SDIO_CHECK_CRC;
  uint32_t response;
  enum Result res;

  res = executeCommand(device,
      SDIO_COMMAND(CMD13_SEND_STATUS, MMCSD_RESPONSE_R1, flags),
      address, &response, true);
  if (res != E_OK)
    return res;

  const uint8_t state = CURRENT_STATE(response);

  if (state == CARD_STANDBY)
  {
    return executeCommand(device,
        SDIO_COMMAND(CMD7_SELECT_CARD, MMCSD_RESPONSE_R1B, flags),
        address, 0, true);
  }
  else if (state != CARD_TRANSFER)
  {
    return res;
  }
  else
    return E_OK;
}
/*----------------------------------------------------------------------------*/
static enum Result startCardSelection(struct MMCSD *device)
{
  const uint32_t address = device->address << 16;
  const uint32_t flags = SDIO_WAIT_DATA | SDIO_CHECK_CRC;

  const enum Result res = executeCommand(device,
      SDIO_COMMAND(CMD7_SELECT_CARD, MMCSD_RESPONSE_R1B, flags),
      address, 0, false);

  if (res != E_BUSY)
  {
    /* Operation must not be completed instantly */
    return res != E_OK ? res : E_INVALID;
  }
  else
    return E_OK;
}
/*----------------------------------------------------------------------------*/
static enum Result startTransfer(struct MMCSD *device)
{
  enum Result res;

  res = ifSetParam(device->interface, IF_SDIO_COMMAND, &device->command);
  if (res != E_OK)
    return res;
  res = ifSetParam(device->interface, IF_SDIO_ARGUMENT, &device->argument);
  if (res != E_OK)
    return res;

  const enum MMCSDCommand code = COMMAND_CODE_VALUE(device->command);
  const size_t length = device->length;
  size_t queued;

  if (code == CMD17_READ_SINGLE_BLOCK || code == CMD18_READ_MULTIPLE_BLOCK)
    queued = ifRead(device->interface, (void *)device->buffer, length);
  else
    queued = ifWrite(device->interface, (const void *)device->buffer, length);

  return queued == length ? E_OK : E_INTERFACE;
}
/*----------------------------------------------------------------------------*/
static enum Result startTransferStateSetup(struct MMCSD *device)
{
  const uint32_t address = device->address << 16;
  const uint32_t flags = SDIO_WAIT_DATA | SDIO_CHECK_CRC;
  const enum Result res = executeCommand(device,
      SDIO_COMMAND(CMD13_SEND_STATUS, MMCSD_RESPONSE_R1, flags),
      address, 0, false);

  /* Operation must not be completed instantly */
  return res == E_BUSY ? E_OK : (res == E_OK ? E_INVALID : res);
}
/*----------------------------------------------------------------------------*/
static enum Result terminateTransfer(struct MMCSD *device)
{
  const uint32_t flags = SDIO_STOP_TRANSFER | SDIO_CHECK_CRC;
  const uint32_t command =
      SDIO_COMMAND(CMD12_STOP_TRANSMISSION, MMCSD_RESPONSE_R1B, flags);
  const enum Result res = executeCommand(device, command, 0, 0, false);

  /* Operation must not be completed instantly */
  return res == E_BUSY ? E_OK : (res == E_OK ? E_INVALID : res);
}
/*----------------------------------------------------------------------------*/
static enum Result transferBuffer(struct MMCSD *device,
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
  const struct MMCSDConfig * const config = configBase;
  struct MMCSD * const device = object;

  device->callback = 0;

  device->interface = config->interface;
  device->position = 0;

  device->sectors = 0;
  device->address = 0;
  device->capacity = MMCSD_SC;
  device->type = CARD_SD_1_0;
  device->crc = config->crc;

  device->state = STATE_IDLE;
  device->blocking = true;

  return initializeCard(device);
}
/*----------------------------------------------------------------------------*/
static void cardDeinit(void *object)
{
  struct MMCSD * const device = object;
  ifSetCallback(device->interface, 0, 0); //FIXME Acquire?
}
/*----------------------------------------------------------------------------*/
static void cardSetCallback(void *object, void (*callback)(void *),
    void *argument)
{
  struct MMCSD * const device = object;

  device->callbackArgument = argument;
  device->callback = callback;
}
/*----------------------------------------------------------------------------*/
static enum Result cardGetParam(void *object, int parameter, void *data)
{
  struct MMCSD * const device = object;

  switch ((enum IfParameter)parameter)
  {
    case IF_POSITION_64:
      *(uint64_t *)data = device->position;
      return E_OK;

    case IF_SIZE_64:
      *(uint64_t *)data = (uint64_t)device->sectors << BLOCK_POW;
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
static enum Result cardSetParam(void *object, int parameter, const void *data)
{
  struct MMCSD * const device = object;

  switch ((enum IfParameter)parameter)
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
      if ((position >> BLOCK_POW) >= (uint64_t)device->sectors)
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
  struct MMCSD * const device = object;

  if (!blocks)
    return 0;

  uint32_t flags = SDIO_DATA_MODE;

  if (blocks > 1)
    flags |= SDIO_AUTO_STOP;
  if (device->crc)
    flags |= SDIO_CHECK_CRC;

  const enum MMCSDCommand code = blocks == 1 ?
      CMD17_READ_SINGLE_BLOCK : CMD18_READ_MULTIPLE_BLOCK;
  const enum MMCSDResponse response = device->mode == SDIO_SPI ?
      MMCSD_RESPONSE_NONE : MMCSD_RESPONSE_R1;
  const uint32_t command = SDIO_COMMAND(code, response, flags);
  const uint32_t argument = (uint32_t)(device->capacity == MMCSD_SC ?
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
  struct MMCSD * const device = object;

  if (!blocks)
    return 0;

  uint32_t flags = SDIO_DATA_MODE | SDIO_WRITE_MODE;

  if (blocks > 1)
    flags |= SDIO_AUTO_STOP;
  if (device->crc)
    flags |= SDIO_CHECK_CRC;

  /* TODO Protect position reading */
  const enum MMCSDCommand code = blocks == 1 ?
      CMD24_WRITE_BLOCK : CMD25_WRITE_MULTIPLE_BLOCK;
  const enum MMCSDResponse response = device->mode == SDIO_SPI ?
      MMCSD_RESPONSE_NONE : MMCSD_RESPONSE_R1;
  const uint32_t command = SDIO_COMMAND(code, response, flags);
  const uint32_t argument = (uint32_t)(device->capacity == MMCSD_SC ?
      device->position : (device->position >> BLOCK_POW));

  const enum Result res = transferBuffer(device, command, argument,
      (uintptr_t)buffer, length);

  return (res == E_OK || res == E_BUSY) ? length : 0;
}
