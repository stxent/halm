/*
 * sdcard.c
 * Copyright (C) 2014 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

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
static enum result executeCommand(struct SdCard *, uint32_t, uint32_t,
    uint32_t *);
static uint32_t extractBits(const uint32_t *, uint16_t, uint16_t);
static enum result identifyCard(struct SdCard *);
static enum result initializeCard(struct SdCard *);
static void processCardSpecificData(struct SdCard *, uint32_t *);
static enum result setTransferState(struct SdCard *);
static enum result stopTransfer(struct SdCard *);
/*----------------------------------------------------------------------------*/
static enum result cardInit(void *, const void *);
static void cardDeinit(void *);
static enum result cardCallback(void *, void (*)(void *), void *);
static enum result cardGet(void *, enum ifOption, void *);
static enum result cardSet(void *, enum ifOption, const void *);
static uint32_t cardRead(void *, uint8_t *, uint32_t);
static uint32_t cardWrite(void *, const uint8_t *, uint32_t);
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
    uint32_t argument, uint32_t *response)
{
  enum result res, status;

  if ((res = ifSet(device->interface, IF_SDIO_COMMAND, &command)) != E_OK)
    return res;
  if ((res = ifSet(device->interface, IF_SDIO_ARGUMENT, &argument)) != E_OK)
    return res;
  if ((res = ifSet(device->interface, IF_SDIO_EXECUTE, 0)) != E_OK)
    return res;

  while ((status = ifGet(device->interface, IF_STATUS, 0)) == E_BUSY)
    barrier();

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
static uint32_t extractBits(const uint32_t *data, uint16_t start, uint16_t end)
{
  const uint16_t index = end >> 5;
  const uint16_t offset = start & 0x1F;
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
      SDIO_RESPONSE_NONE, SDIO_INITIALIZE), 0, 0);
  if (res != E_OK && res != E_IDLE)
    return res;

  /* Start initialization and detect card type */
  res = executeCommand(device, SDIO_COMMAND(CMD_SEND_IF_COND,
      SDIO_RESPONSE_SHORT, 0), CONDITION_PATTERN, response);
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

  const enum sdioResponse responseType = device->native ?
      SDIO_RESPONSE_SHORT : SDIO_RESPONSE_NONE;
  const uint32_t crcStatus = device->crc ? SDIO_CHECK_CRC : 0;
  uint32_t ocr = 0;

  if (device->type == SDCARD_2_0)
    ocr |= OCR_HCS;
  if (device->native)
    ocr |= OCR_VOLTAGE_MASK;

  /* Wait till card becomes ready */
  for (uint8_t counter = 100; counter; --counter)
  {
    res = executeCommand(device, SDIO_COMMAND(CMD_APP_CMD,
        responseType, 0), 0, 0);
    if (res != E_OK && res != E_IDLE)
      break;

    res = executeCommand(device, SDIO_COMMAND(ACMD_SD_SEND_OP_COND,
        responseType, 0), ocr, device->native ? response : 0);
    if (device->native)
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

  /* Enable optional integrity checking */
  if (!device->native && device->crc)
  {
    res = executeCommand(device, SDIO_COMMAND(CMD_CRC_ON_OFF,
        SDIO_RESPONSE_SHORT, 0), CRC_ENABLED, 0);
    if (res != E_OK)
      return res;
  }

  /* Read card capacity information when SPI mode is used */
  if (!device->native && device->type == SDCARD_2_0)
  {
    res = executeCommand(device, SDIO_COMMAND(CMD_READ_OCR,
        SDIO_RESPONSE_SHORT, crcStatus), 0, response);
    if (res != E_OK)
      return res;

    if (response[0] & OCR_CCS)
      device->capacity = SDCARD_SDHC;
  }

  /* Read CID and RCA information */
  if (device->native)
  {
    res = executeCommand(device, SDIO_COMMAND(CMD_ALL_SEND_CID,
        SDIO_RESPONSE_LONG, crcStatus), 0, response);
    if (res != E_OK)
      return res;

    res = executeCommand(device, SDIO_COMMAND(CMD_SEND_RELATIVE_ADDR,
        SDIO_RESPONSE_SHORT, crcStatus), 0, response);
    if (res != E_OK)
      return res;

    device->address = response[0] >> 16;
  }

  const uint32_t address = (uint32_t)device->address << 16;

  /* Read and process CSD register */
  res = executeCommand(device, SDIO_COMMAND(CMD_SEND_CSD,
      SDIO_RESPONSE_LONG, crcStatus), address, response);
  if (res != E_OK)
    return res;
  processCardSpecificData(device, response);

  /* Configure block length and bus width */
  if (device->native)
  {
    if ((res = setTransferState(device)) != E_OK)
        return res;

    res = executeCommand(device, SDIO_COMMAND(CMD_SET_BLOCKLEN,
        SDIO_RESPONSE_SHORT, crcStatus), 1 << BLOCK_POW, 0);
    if (res != E_OK)
      return res;

    if ((res = ifGet(device->interface, IF_SDIO_MODE, response)) != E_OK)
      return res;

    const uint8_t width = response[0] == SDIO_1BIT ?
        BUS_WIDTH_1BIT : BUS_WIDTH_4BIT;

    res = executeCommand(device, SDIO_COMMAND(CMD_APP_CMD,
        SDIO_RESPONSE_SHORT, crcStatus), address, 0);
    if (res != E_OK)
      return res;

    res = executeCommand(device, SDIO_COMMAND(ACMD_SET_BUS_WIDTH,
        SDIO_RESPONSE_SHORT, crcStatus), width, 0);
    if (res != E_OK)
      return res;
  }

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static enum result initializeCard(struct SdCard *device)
{
  const uint32_t lowRate = ENUM_RATE;
  uint32_t rate;
  enum result res;

  /* Lock the interface */
  ifSet(device->interface, IF_ACQUIRE, 0);

  if ((res = ifGet(device->interface, IF_RATE, &rate)) != E_OK)
    return res;
  if (rate > WORK_RATE)
    return E_VALUE;

  /* Set low data rate for enumeration purposes */
  if ((res = ifSet(device->interface, IF_RATE, &lowRate)) != E_OK)
    return res;

  /* Initialize memory card */
  res = identifyCard(device);

  /* Restore original interface rate */
  ifSet(device->interface, IF_RATE, &rate);

  /* Release the interface */
  ifSet(device->interface, IF_RELEASE, 0);

  return res;
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
  const uint32_t address = (uint32_t)device->address << 16;
  const uint32_t flags = SDIO_WAIT_DATA | (device->crc ? SDIO_CHECK_CRC : 0);
  uint32_t response;
  enum result res;

  res = executeCommand(device, SDIO_COMMAND(CMD_SEND_STATUS,
      SDIO_RESPONSE_SHORT, flags), address, &response);
  if (res != E_OK)
    return res;

  const uint8_t state = CURRENT_STATE(response);

  if (state == CARD_STANDBY)
  {
    return executeCommand(device, SDIO_COMMAND(CMD_SELECT_CARD,
        SDIO_RESPONSE_SHORT, flags), address, 0);
  }
  else if (state != CARD_TRANSFER)
  {
    return res;
  }
  else
    return E_OK;
}
/*----------------------------------------------------------------------------*/
static enum result stopTransfer(struct SdCard *device)
{
  const uint32_t flags = (device->crc ? SDIO_CHECK_CRC : 0)
      | SDIO_STOP_TRANSFER;
  enum result res;

  res = executeCommand(device, SDIO_COMMAND(CMD_STOP_TRANSMISSION,
      SDIO_RESPONSE_SHORT, flags), 0, 0);

  return res;
}
/*----------------------------------------------------------------------------*/
static enum result cardInit(void *object, const void *configBase)
{
  const struct SdCardConfig * const config = configBase;
  struct SdCard * const device = object;
  uint32_t mode;
  enum result res;

  device->address = 0;
  device->blockCount = 0;
  device->capacity = SDCARD_SDSC;
  device->crc = config->crc;
  device->interface = config->interface;
  device->position = 0;
  device->type = SDCARD_1_0;

  /* Get interface type */
  if ((res = ifGet(device->interface, IF_SDIO_MODE, &mode)) != E_OK)
    return res;
  device->native = mode != SDIO_SPI;

  return initializeCard(device);
}
/*----------------------------------------------------------------------------*/
static void cardDeinit(void *object __attribute__((unused)))
{

}
/*----------------------------------------------------------------------------*/
static enum result cardCallback(void *object __attribute__((unused)),
    void (*callback)(void *) __attribute__((unused)),
    void *argument __attribute__((unused)))
{
  return E_ERROR;
}
/*----------------------------------------------------------------------------*/
static enum result cardGet(void *object, enum ifOption option, void *data)
{
  struct SdCard * const device = object;

  switch (option)
  {
    case IF_ADDRESS:
      *(uint64_t *)data = device->position;
      return E_OK;

    case IF_SIZE:
      *(uint64_t *)data = (uint64_t)device->blockCount << BLOCK_POW;
      return E_OK;

    default:
      return E_ERROR;
  }
}
/*----------------------------------------------------------------------------*/
static enum result cardSet(void *object, enum ifOption option,
    const void *data)
{
  struct SdCard * const device = object;

  switch (option)
  {
    case IF_ADDRESS:
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

    default:
      return E_ERROR;
  }
}
/*----------------------------------------------------------------------------*/
static uint32_t cardRead(void *object, uint8_t *buffer, uint32_t length)
{
  const uint32_t blocks = length >> BLOCK_POW;
  struct SdCard * const device = object;
  enum result res;

  if (!blocks)
    return 0;

  ifSet(device->interface, IF_ACQUIRE, 0);

  if (device->native && (res = setTransferState(device)) != E_OK)
    goto exit;

  uint32_t flags = SDIO_DATA_MODE;

  if (blocks > 1)
    flags |= SDIO_AUTO_STOP;
  if (device->crc)
    flags |= SDIO_CHECK_CRC;

  const enum sdioCommand code = blocks == 1 ?
      CMD_READ_SINGLE_BLOCK : CMD_READ_MULTIPLE_BLOCK;
  const enum sdioResponse response = device->native ?
      SDIO_RESPONSE_SHORT : SDIO_RESPONSE_NONE;
  const uint32_t command = SDIO_COMMAND(code, response, flags);
  const uint32_t argument = (uint32_t)(device->capacity == SDCARD_SDSC ?
      device->position : device->position >> BLOCK_POW);

  if ((res = ifSet(device->interface, IF_SDIO_COMMAND, &command)) != E_OK)
    goto exit;
  if ((res = ifSet(device->interface, IF_SDIO_ARGUMENT, &argument)) != E_OK)
    goto exit;

  if (ifRead(device->interface, buffer, length) != length)
  {
    res = E_INTERFACE;
    goto exit;
  }

  while ((res = ifGet(device->interface, IF_STATUS, 0)) == E_BUSY)
    barrier();

  if (res != E_OK && blocks > 1)
    stopTransfer(device);

exit:
  ifSet(device->interface, IF_RELEASE, 0);
  return res == E_OK ? length : 0;
}
/*----------------------------------------------------------------------------*/
static uint32_t cardWrite(void *object, const uint8_t *buffer, uint32_t length)
{
  const uint32_t blocks = length >> BLOCK_POW;
  struct SdCard * const device = object;
  enum result res;

  if (!blocks)
    return 0;

  ifSet(device->interface, IF_ACQUIRE, 0);

  if (device->native && (res = setTransferState(device)) != E_OK)
    goto exit;

  uint32_t flags = SDIO_DATA_MODE | SDIO_WRITE_MODE;

  if (blocks > 1)
    flags |= SDIO_AUTO_STOP;
  if (device->crc)
    flags |= SDIO_CHECK_CRC;

  const enum sdioCommand code = blocks == 1 ?
      CMD_WRITE_BLOCK : CMD_WRITE_MULTIPLE_BLOCK;
  const enum sdioResponse response = device->native ?
      SDIO_RESPONSE_SHORT : SDIO_RESPONSE_NONE;
  const uint32_t command = SDIO_COMMAND(code, response, flags);
  const uint32_t argument = (uint32_t)(device->capacity == SDCARD_SDSC ?
      device->position : device->position >> BLOCK_POW);

  if ((res = ifSet(device->interface, IF_SDIO_COMMAND, &command)) != E_OK)
    goto exit;
  if ((res = ifSet(device->interface, IF_SDIO_ARGUMENT, &argument)) != E_OK)
    goto exit;

  if (ifWrite(device->interface, buffer, length) != length)
  {
    res = E_INTERFACE;
    goto exit;
  }

  while ((res = ifGet(device->interface, IF_STATUS, 0)) == E_BUSY)
    barrier();

  if (res != E_OK && blocks > 1)
    stopTransfer(device);

exit:
  ifSet(device->interface, IF_RELEASE, 0);
  return res == E_OK ? length : 0;
}
