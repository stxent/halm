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
static enum Result initStepMmcSetHighSpeed(struct MMCSD *, enum MMCSDSpeed);
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
static enum Result eraseSectorGroup(struct MMCSD *, uint32_t);
static enum Result executeCommand(struct MMCSD *, uint32_t, uint32_t,
    uint32_t *, bool);
static bool extractBit(const uint32_t *, unsigned int);
static uint32_t extractBits(const uint32_t *, unsigned int, unsigned int);
static enum Result identifyCard(struct MMCSD *);
static enum Result initializeCard(struct MMCSD *);
static void interruptHandler(void *);
static enum Result isCardReady(struct MMCSD *);
static bool onCardSelectionFinished(struct MMCSD *);
static bool onTransferStateSetupFinished(struct MMCSD *);
static void parseCardSpecificData(struct MMCSD *, const uint32_t *);
static enum Result setTransferState(struct MMCSD *);
static enum Result startCardSelection(struct MMCSD *);
static enum Result startTransfer(struct MMCSD *);
static enum Result startTransferStateSetup(struct MMCSD *);
static enum Result terminateTransfer(struct MMCSD *);
static enum Result transferBuffer(struct MMCSD *, uint32_t, uint32_t,
    uintptr_t, size_t);
/*----------------------------------------------------------------------------*/
static enum Result cardInit(void *, const void *);
static void cardSetCallback(void *, void (*)(void *), void *);
static enum Result cardGetParam(void *, int, void *);
static enum Result cardSetParam(void *, int, const void *);
static size_t cardRead(void *, void *, size_t);
static size_t cardWrite(void *, const void *, size_t);
/*----------------------------------------------------------------------------*/
const struct InterfaceClass * const MMCSD = &(const struct InterfaceClass){
    .size = sizeof(struct MMCSD),
    .init = cardInit,
    .deinit = NULL, /* Default destructor */

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
      CMD59_CRC_ENABLED, 0, true);
}
/*----------------------------------------------------------------------------*/
static enum Result initStepMmcReadOCR(struct MMCSD *device)
{
  uint32_t ocr = 0;
  enum Result res;

  for (unsigned int counter = 100; counter; --counter)
  {
    if (!ocr)
    {
      res = executeCommand(device,
          SDIO_COMMAND(CMD1_SEND_OP_COND, MMCSD_RESPONSE_R3, 0),
          0, &ocr, true);

      if (res == E_OK)
      {
        ocr = OCR_HCS | (ocr & OCR_VOLTAGE_MASK_2V7_3V6);
      }
    }

    if (ocr)
    {
      uint32_t response;

      res = executeCommand(device,
          SDIO_COMMAND(CMD1_SEND_OP_COND, MMCSD_RESPONSE_R3, 0),
	      ocr, &response, true);

      if (res == E_OK)
      {
        /* Busy bit is active low */
        if (response & OCR_BUSY)
        {
          if (response & OCR_MMC_SECTOR_MODE)
          {
            /* Card capacity is greater than 2GB */
            device->info.capacityType = CAPACITY_HC;
          }
          else
          {
            /* Card capacity is less then or equal to 2GB */
            device->info.capacityType = CAPACITY_SC;
          }

          device->info.cardType = CARD_MMC;
        }
        else
        {
          res = E_BUSY;
        }
      }
    }

    if (res != E_BUSY)
      break;

    /* TODO Remove delay */
    mdelay(10);
  }

  return res;
}
/*----------------------------------------------------------------------------*/
static enum Result initStepMmcSetBusWidth(struct MMCSD *device)
{
  uint32_t mode = MMC_BUS_WIDTH_PATTERN;

  switch (device->mode)
  {
    case SDIO_4BIT:
      mode |= MMC_BUS_WIDTH_4BIT;
      break;

    case SDIO_8BIT:
      mode |= MMC_BUS_WIDTH_8BIT;
      break;

    default:
      mode |= MMC_BUS_WIDTH_1BIT;
      break;
  }

  return executeCommand(device,
      SDIO_COMMAND(CMD6_SWITCH, MMCSD_RESPONSE_R1B, SDIO_CHECK_CRC),
      mode, 0, true);
}
/*----------------------------------------------------------------------------*/
static enum Result initStepMmcSetHighSpeed(struct MMCSD *device,
    enum MMCSDSpeed speed)
{
  uint32_t mode = MMC_HS_TIMING_PATTERN;

  switch (speed)
  {
    case SPEED_HS:
      mode |= MMC_HS_TIMING_HS;
      break;

    case SPEED_HS200:
      mode |= MMC_HS_TIMING_HS200;
      break;

    case SPEED_HS400:
      mode |= MMC_HS_TIMING_HS400;
      break;

    default:
      mode |= MMC_HS_TIMING_COMPATIBLE;
      break;
  }

  return executeCommand(device,
      SDIO_COMMAND(CMD6_SWITCH, MMCSD_RESPONSE_R1B, SDIO_CHECK_CRC),
      mode, 0, true);
}
/*----------------------------------------------------------------------------*/
static enum Result initStepReadCondition(struct MMCSD *device)
{
  uint32_t response;
  const enum Result res = executeCommand(device,
      SDIO_COMMAND(CMD8_SEND_IF_COND, MMCSD_RESPONSE_R7, 0),
      CMD8_CONDITION_PATTERN, &response, true);

  if (res == E_OK || res == E_IDLE)
  {
    /* Response should be equal to the command argument */
    if (response != CMD8_CONDITION_PATTERN)
      return E_DEVICE;

    device->info.cardType = CARD_SD_2_0;
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
      (device->info.cardAddress << 16), response, true);

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
    do
    {
      status = ifGetParam(device->interface, IF_STATUS, NULL);
      barrier();
    }
    while (status == E_BUSY);
  }
  else
    return E_INTERFACE;

  /* Process SEC_COUNT parameter [215:212] */
  uint32_t sectors;
  memcpy(&sectors, &csd[212], sizeof(sectors));
  device->info.sectorCount = fromLittleEndian32(sectors);

  /* Process HC_ERASE_GRP_SIZE parameter [224] */
  if (csd[224] != 0)
  {
    device->info.eraseGroupSize = csd[224] * ((512 * 1024) >> BLOCK_POW);
  }

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
    device->info.cardAddress = response >> 16;
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

  if (device->info.cardType == CARD_SD_2_0)
    ocr |= OCR_HCS;
  if (device->mode != SDIO_SPI)
    ocr |= OCR_VOLTAGE_MASK_2V7_3V6;

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
          device->info.capacityType = CAPACITY_HC;
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
      (device->info.cardAddress << 16), 0, true);
  if (res != E_OK)
    return res;

  const uint8_t busMode = device->mode == SDIO_4BIT ?
      ACMD6_BUS_WIDTH_4BIT : ACMD6_BUS_WIDTH_1BIT;

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
    device->info.cardAddress = address;
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
    device->info.capacityType = CAPACITY_HC;
  return res;
}
/*----------------------------------------------------------------------------*/
static enum Result eraseSectorGroup(struct MMCSD *device, uint32_t sector)
{
  const uint32_t argument = device->info.capacityType == CAPACITY_SC ?
      (sector << BLOCK_POW) : sector;
  enum Result res;

  /* Lock the bus */
  ifSetParam(device->interface, IF_ACQUIRE, NULL);

  res = executeCommand(device,
      SDIO_COMMAND(CMD35_ERASE_GROUP_START, MMCSD_RESPONSE_R1, SDIO_CHECK_CRC),
      argument, 0, true);
  if (res != E_OK)
    goto error;

  res = executeCommand(device,
      SDIO_COMMAND(CMD36_ERASE_GROUP_END, MMCSD_RESPONSE_R1, SDIO_CHECK_CRC),
      argument, 0, true);
  if (res != E_OK)
    goto error;

  res = executeCommand(device,
      SDIO_COMMAND(CMD38_ERASE, MMCSD_RESPONSE_R1B, SDIO_CHECK_CRC),
      0, 0, true);
  if (res != E_OK)
    goto error;

error:
  /* Release the bus */
  ifSetParam(device->interface, IF_RELEASE, NULL);

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

  enum Result status = ifSetParam(device->interface, IF_SDIO_EXECUTE, NULL);

  if (status == E_BUSY)
  {
    if (!await)
      return E_BUSY;

    do
    {
      status = ifGetParam(device->interface, IF_STATUS, NULL);
      barrier();
    }
    while (status == E_BUSY);
  }

  if (status != E_OK && status != E_IDLE)
    return status;

  if (response != NULL)
  {
    res = ifGetParam(device->interface, IF_SDIO_RESPONSE, response);
    if (res != E_OK)
      return res;
  }

  return status;
}
/*----------------------------------------------------------------------------*/
static bool extractBit(const uint32_t *data, unsigned int position)
{
  const unsigned int index = position >> 5;
  const unsigned int offset = position & 0x1F;

  return (data[index] & (1UL << offset)) != 0;
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
  udelay(CMD0_IDLE_DELAY);

  /* Try to initialize as MMC */
  if ((res = initStepMmcReadOCR(device)) != E_OK)
  {
    /* Send Reset command */
    if ((res = initStepSendReset(device)) != E_OK)
      return res;
    udelay(CMD0_IDLE_DELAY);

    /* Card is not MMC, try to initialize as SD */
    if ((res = initStepReadCondition(device)) != E_OK)
      return res;

    res = initStepSdReadOCR(device, device->mode == SDIO_SPI ?
        MMCSD_RESPONSE_NONE : MMCSD_RESPONSE_R1);
    if (res != E_OK)
      return res;
  }

  /* Enable optional integrity checking */
  if (device->mode == SDIO_SPI && device->crc)
  {
    if ((res = initStepEnableCrc(device)) != E_OK)
      return res;
  }

  /* Read card capacity information when SPI mode is used */
  if (device->mode == SDIO_SPI && device->info.cardType == CARD_SD_2_0)
  {
    if ((res = initStepSpiReadOCR(device)) != E_OK)
      return res;
  }

  /* Read CID and RCA information */
  if (device->mode != SDIO_SPI)
  {
    if ((res = initStepReadCID(device)) != E_OK)
      return res;

    if (device->info.cardType >= CARD_MMC)
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

    if (device->info.cardType == CARD_MMC_4_0)
    {
      if ((res = initStepMmcSetBusWidth(device)) != E_OK)
        return res;

      /* Enable high-speed mode by default */
      if ((res = initStepMmcSetHighSpeed(device, SPEED_HS)) != E_OK)
        return res;
    }
    else
    {
      if ((res = initStepSdSetBusWidth(device)) != E_OK)
        return res;
    }
  }

  /* Read Extended CSD register of the MMC */
  if (device->info.cardType == CARD_MMC_4_0
      && device->info.capacityType == CAPACITY_HC)
  {
    if ((res = initStepReadExtCSD(device)) != E_OK)
      return res;
  }

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static enum Result initializeCard(struct MMCSD *device)
{
  static const uint32_t enumRate = RATE_ENUMERATION;
  uint32_t workRate;
  enum Result res;

  /* Get the type of the bus */
  res = ifGetParam(device->interface, IF_SDIO_MODE, &device->mode);
  if (res != E_OK)
    return res;

  /* Lock the bus */
  ifSetParam(device->interface, IF_ACQUIRE, NULL);

  /* Check interface capabilities and select zero-copy mode */
  res = ifSetParam(device->interface, IF_ZEROCOPY, NULL);
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
  /* Release the bus */
  ifSetCallback(device->interface, NULL, NULL);
  ifSetParam(device->interface, IF_RELEASE, NULL);

  return res;
}
/*----------------------------------------------------------------------------*/
static void interruptHandler(void *object)
{
  struct MMCSD * const device = object;
  bool event = false;

  switch ((enum State)device->transfer.state)
  {
    case STATE_GET_STATUS:
      if (!onTransferStateSetupFinished(device))
      {
        event = true;
        device->transfer.state = STATE_ERROR;
      }
      break;

    case STATE_SELECT_CARD:
      if (!onCardSelectionFinished(device))
      {
        event = true;
        device->transfer.state = STATE_ERROR;
      }
      break;

    case STATE_TRANSFER:
    {
      const enum Result res = ifGetParam(device->interface, IF_STATUS, NULL);

      if (res != E_OK)
      {
        device->transfer.state = STATE_HALT;
      }
      else
      {
        event = true;
        device->transfer.state = STATE_IDLE;
      }

      if (device->transfer.state == STATE_HALT)
      {
        if (terminateTransfer(device) != E_OK)
        {
          event = true;
          device->transfer.state = STATE_ERROR;
        }
      }
      break;
    }

    case STATE_STOP:
    {
      const enum Result res = ifGetParam(device->interface, IF_STATUS, NULL);

      event = true;
      device->transfer.state = res == E_OK ? STATE_IDLE : STATE_ERROR;
      break;
    }

    case STATE_HALT:
      event = true;
      device->transfer.state = STATE_ERROR;
      break;

    default:
      break;
  }

  if (event)
  {
    /* Release the bus */
    ifSetCallback(device->interface, NULL, NULL);
    ifSetParam(device->interface, IF_RELEASE, NULL);

    if (device->callback != NULL)
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
static bool onCardSelectionFinished(struct MMCSD *device)
{
  device->transfer.state = STATE_TRANSFER;
  return startTransfer(device) == E_OK;
}
/*----------------------------------------------------------------------------*/
static bool onTransferStateSetupFinished(struct MMCSD *device)
{
  bool completed = true;

  switch (isCardReady(device))
  {
    case E_OK:
      device->transfer.state = STATE_TRANSFER;

      if (startTransfer(device) != E_OK)
        completed = false;
      break;

    case E_IDLE:
      device->transfer.state = STATE_SELECT_CARD;

      if (startCardSelection(device) != E_OK)
        completed = false;
      break;

    default:
      completed = false;
      break;
  }

  return completed;
}
/*----------------------------------------------------------------------------*/
static void parseCardSpecificData(struct MMCSD *device,
    const uint32_t *response)
{
  if (device->info.cardType == CARD_MMC)
  {
    const uint8_t specVers = extractBits(response, 122, 125);

    if (specVers >= 4)
      device->info.cardType = CARD_MMC_4_0;
  }

  /* Erase group size */

  if (device->info.cardType == CARD_MMC)
  {
    const uint32_t eraseGroupSize = extractBits(response, 42, 46);
    const uint32_t eraseGroupMult = extractBits(response, 37, 41);

    device->info.eraseGroupSize = (eraseGroupSize + 1) * (eraseGroupMult + 1);
  }
  else if (device->info.capacityType == CAPACITY_SC)
  {
    const uint32_t eraseSectorSize = extractBits(response, 39, 45) + 1;
    const bool eraseBlockEnable = extractBit(response, 46);

    device->info.eraseGroupSize = eraseBlockEnable ? 1 : eraseSectorSize;
    device->info.eraseGroupSize <<= 9;
  }

  /* Sector count */

  if (device->info.capacityType == CAPACITY_SC)
  {
    const uint32_t blockLength = extractBits(response, 80, 83);
    const uint32_t deviceSize = extractBits(response, 62, 73) + 1;
    const uint32_t sizeMult = extractBits(response, 47, 49);

    device->info.sectorCount = (deviceSize << (sizeMult + 2));
    device->info.sectorCount <<= blockLength - 9;
  }
  else if (device->info.cardType <= CARD_SD_2_0)
  {
    const uint32_t deviceSize = extractBits(response, 48, 69) + 1;
    device->info.sectorCount = deviceSize << 10;
  }
}
/*----------------------------------------------------------------------------*/
static enum Result setTransferState(struct MMCSD *device)
{
  /* Relative card address should be initialized */
  const uint32_t address = device->info.cardAddress << 16;
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
  const uint32_t address = device->info.cardAddress << 16;
  const uint32_t flags = SDIO_WAIT_DATA | SDIO_CHECK_CRC;

  const enum Result res = executeCommand(device,
      SDIO_COMMAND(CMD7_SELECT_CARD, MMCSD_RESPONSE_R1B, flags),
      address, 0, false);

  if (res == E_OK)
  {
    return onCardSelectionFinished(device) ? E_OK : E_INTERFACE;
  }
  else
  {
    return res != E_BUSY ? res : E_OK;
  }
}
/*----------------------------------------------------------------------------*/
static enum Result startTransfer(struct MMCSD *device)
{
  enum Result res;

  res = ifSetParam(device->interface, IF_SDIO_COMMAND,
      &device->transfer.command);
  if (res != E_OK)
    return res;

  res = ifSetParam(device->interface, IF_SDIO_ARGUMENT,
      &device->transfer.argument);
  if (res != E_OK)
    return res;

  const enum MMCSDCommand code = COMMAND_CODE_VALUE(device->transfer.command);
  size_t queued;

  if (code == CMD17_READ_SINGLE_BLOCK || code == CMD18_READ_MULTIPLE_BLOCK)
  {
    queued = ifRead(device->interface, (void *)device->transfer.buffer,
        device->transfer.length);
  }
  else
  {
    queued = ifWrite(device->interface, (const void *)device->transfer.buffer,
        device->transfer.length);
  }

  return queued == device->transfer.length ? E_OK : E_INTERFACE;
}
/*----------------------------------------------------------------------------*/
static enum Result startTransferStateSetup(struct MMCSD *device)
{
  const uint32_t address = device->info.cardAddress << 16;
  const uint32_t flags = SDIO_WAIT_DATA | SDIO_CHECK_CRC;
  const enum Result res = executeCommand(device,
      SDIO_COMMAND(CMD13_SEND_STATUS, MMCSD_RESPONSE_R1, flags),
      address, 0, false);

  if (res == E_OK)
  {
    return onTransferStateSetupFinished(device) ? E_OK : E_INTERFACE;
  }
  else
  {
    return res != E_BUSY ? res : E_OK;
  }
}
/*----------------------------------------------------------------------------*/
static enum Result terminateTransfer(struct MMCSD *device)
{
  const uint32_t flags = SDIO_STOP_TRANSFER | SDIO_CHECK_CRC;
  const uint32_t command =
      SDIO_COMMAND(CMD12_STOP_TRANSMISSION, MMCSD_RESPONSE_R1B, flags);
  const enum Result res = executeCommand(device, command, 0, 0, false);

  if (res != E_BUSY)
  {
    /* Operation must not be completed instantly */
    return res == E_OK ? E_INVALID : res;
  }
  else
    return E_OK;
}
/*----------------------------------------------------------------------------*/
static enum Result transferBuffer(struct MMCSD *device,
    uint32_t command, uint32_t argument, uintptr_t buffer, size_t length)
{
  enum Result res;

  ifSetParam(device->interface, IF_ACQUIRE, NULL);
  ifSetParam(device->interface, IF_ZEROCOPY, NULL);
  ifSetCallback(device->interface, interruptHandler, device);

  device->transfer.argument = argument;
  device->transfer.command = command;
  device->transfer.buffer = buffer;
  device->transfer.length = length;

  if (device->mode != SDIO_SPI)
  {
    device->transfer.state = STATE_GET_STATUS;
    res = startTransferStateSetup(device);
  }
  else
  {
    device->transfer.state = STATE_TRANSFER;
    res = startTransfer(device);
  }

  if (res != E_OK)
  {
    device->transfer.state = STATE_ERROR;

    /* Release the bus */
    ifSetCallback(device->interface, NULL, NULL);
    ifSetParam(device->interface, IF_RELEASE, NULL);

    if (device->callback != NULL)
      device->callback(device->callbackArgument);

    return res;
  }

  if (device->blocking)
  {
    while (device->transfer.state != STATE_IDLE
        && device->transfer.state != STATE_ERROR)
    {
      barrier();
    }

    res = device->transfer.state == STATE_ERROR ? E_INTERFACE : E_OK;
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

  device->callback = NULL;

  device->interface = config->interface;
  device->transfer.position = 0;

  device->info.sectorCount = 0;
  device->info.cardAddress = 0;
  device->info.eraseGroupSize = 0;
  device->info.capacityType = CAPACITY_SC;
  device->info.cardType = CARD_SD;
  device->crc = config->crc;

  device->transfer.state = STATE_IDLE;
  device->blocking = true;

  return initializeCard(device);
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

  switch ((enum MMCSDParameter)parameter)
  {
    case IF_MMCSD_ERASE_GROUP_SIZE:
      *(uint32_t *)data = (uint32_t)device->info.eraseGroupSize << BLOCK_POW;
      return E_OK;

    default:
      break;
  }

  switch ((enum IfParameter)parameter)
  {
    case IF_POSITION:
      if (device->transfer.position <= UINT32_MAX)
      {
        *(uint32_t *)data = (uint32_t)device->transfer.position;
        return E_OK;
      }
      else
        return E_MEMORY;

    case IF_POSITION_64:
      *(uint64_t *)data = device->transfer.position;
      return E_OK;

    case IF_SIZE:
      if (((uint64_t)device->info.sectorCount << BLOCK_POW) <= UINT32_MAX)
      {
        *(uint32_t *)data = device->info.sectorCount << BLOCK_POW;
        return E_OK;
      }
      else
        return E_MEMORY;

    case IF_SIZE_64:
      *(uint64_t *)data = (uint64_t)device->info.sectorCount << BLOCK_POW;
      return E_OK;

    case IF_STATUS:
    {
      switch ((enum State)device->transfer.state)
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

  switch ((enum MMCSDParameter)parameter)
  {
    case IF_MMCSD_ERASE:
    {
      const uint32_t position = *(const uint32_t *)data;

      /* Check address alignment */
      if (position & ((device->info.eraseGroupSize << BLOCK_POW) - 1))
        return E_VALUE;

      /* Check address range */
      if ((position >> BLOCK_POW) >= device->info.sectorCount)
        return E_VALUE;

      return eraseSectorGroup(device, position >> BLOCK_POW);
    }

    case IF_MMCSD_ERASE_64:
    {
      const uint64_t position = *(const uint64_t *)data;

      /* Check address alignment */
      if (position & ((device->info.eraseGroupSize << BLOCK_POW) - 1))
        return E_VALUE;

      /* Check address range */
      if ((position >> BLOCK_POW) >= (uint64_t)device->info.sectorCount)
        return E_VALUE;

      return eraseSectorGroup(device, (uint32_t)(position >> BLOCK_POW));
    }

    default:
      break;
  }

  switch ((enum IfParameter)parameter)
  {
    case IF_BLOCKING:
      device->blocking = true;
      return E_OK;

    case IF_POSITION:
    {
      const uint32_t position = *(const uint32_t *)data;

      /* Check address alignment */
      if (position & MASK(BLOCK_POW))
        return E_VALUE;

      /* Check address range */
      if ((position >> BLOCK_POW) >= device->info.sectorCount)
        return E_VALUE;

      device->transfer.position = position;
      return E_OK;
    }

    case IF_POSITION_64:
    {
      const uint64_t position = *(const uint64_t *)data;

      /* Check address alignment */
      if (position & MASK(BLOCK_POW))
        return E_VALUE;

      /* Check address range */
      if ((position >> BLOCK_POW) >= (uint64_t)device->info.sectorCount)
        return E_VALUE;

      device->transfer.position = position;
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
  const uint32_t argument = device->info.capacityType == CAPACITY_SC ?
      device->transfer.position : (device->transfer.position >> BLOCK_POW);
  const uint32_t command = SDIO_COMMAND(code, response, flags);

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
  const uint32_t argument = device->info.capacityType == CAPACITY_SC ?
      device->transfer.position : (device->transfer.position >> BLOCK_POW);
  const uint32_t command = SDIO_COMMAND(code, response, flags);

  const enum Result res = transferBuffer(device, command, argument,
      (uintptr_t)buffer, length);

  return (res == E_OK || res == E_BUSY) ? length : 0;
}
