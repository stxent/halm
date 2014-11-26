/*
 * sdcard.c
 * Copyright (C) 2014 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <assert.h>
#include <bits.h>
#include <delay.h>
#include <memory.h>
#include <modules/sdio.h>
#include <modules/sdio_defs.h>
#include <platform/sdcard.h>
/*----------------------------------------------------------------------------*/
#define BLOCK_POW       9
#define TOKEN_DATA_MASK 0x1F
/*----------------------------------------------------------------------------*/
#define OCR_CCS         BIT(30) /* Card Capacity Status */
#define OCR_HCS         BIT(30) /* Host Capacity Support */
/*----------------------------------------------------------------------------*/
static enum result executeCommand(struct SdCard *, uint32_t, uint32_t,
    uint32_t *);
static uint32_t extractBits(uint32_t *, uint16_t, uint16_t);
static enum result initializeCard(struct SdCard *);
static void processCardSpecificData(struct SdCard *, uint32_t *);
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

  while ((status = ifGet(device->interface, IF_STATUS, 0)) == E_BUSY);

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
static uint32_t extractBits(uint32_t *data, uint16_t start, uint16_t end)
{
  const uint32_t index = end >> 5;
  const uint32_t offset = start & 0x1F;
  uint32_t value;

  if (index == start >> 5)
    value = data[index] >> offset;
  else
    value = (data[index] << (32 - offset)) | (data[start >> 5] >> offset);

  return value & MASK(end - start + 1);
}
/*----------------------------------------------------------------------------*/
static enum result initializeCard(struct SdCard *device)
{
  uint32_t response[4];
  enum sdioMode mode;
  enum result res;

  if ((res = ifGet(device->interface, IF_SDIO_MODE, response)) != E_OK)
    return res;
  mode = response[0];

  /* Send reset command */
  res = executeCommand(device, SDIO_COMMAND(CMD_GO_IDLE_STATE,
      SDIO_RESPONSE_NONE, SDIO_INITIALIZE), 0, 0);
  if (res != E_OK && res != E_IDLE)
    return res;

  /* Start initialization and detect card type */
  res = executeCommand(device, SDIO_COMMAND(CMD_SEND_IF_COND,
      SDIO_RESPONSE_SHORT, 0), 0x000001AA, response);
  if (res == E_OK || res == E_IDLE)
  {
    //TODO Remove magic numbers
    if (response[0] != 0x000001AA)
      return E_DEVICE; /* Pattern mismatched */

    device->type = SDCARD_2_0;
  }
  else if (res != E_INVALID) /* Not an unsupported command */
  {
    return res;
  }

  /* Wait till card becomes ready */
  const enum sdioResponse initResponseType =
      mode == SDIO_SPI ? SDIO_RESPONSE_NONE : SDIO_RESPONSE_SHORT;
  for (uint8_t counter = 100; counter; --counter)
  {
    res = executeCommand(device, SDIO_COMMAND(CMD_APP_CMD,
        initResponseType, 0), 0, 0);
    if (res != E_OK && res != E_IDLE)
      break;

    res = executeCommand(device, SDIO_COMMAND(ACMD_SD_SEND_OP_COND,
        initResponseType, 0), OCR_HCS, mode != SDIO_SPI ? response : 0);
    if (res != E_IDLE)
      break;

    /* TODO Remove delay */
    mdelay(10);
  }
  if (res != E_OK)
  {
    /* Check card capacity information */
    if (mode != SDIO_SPI && (response[0] & OCR_CCS))
      device->capacity = SDCARD_SDHC;

    return res;
  }

  /* Read card capacity information when SPI mode is used */
  if (mode == SDIO_SPI && device->type == SDCARD_2_0)
  {
    res = executeCommand(device, SDIO_COMMAND(CMD_READ_OCR,
        SDIO_RESPONSE_SHORT, 0), 0, response);
    if (res != E_OK)
      return res;

    if (response[0] & OCR_CCS)
      device->capacity = SDCARD_SDHC;
  }

  res = executeCommand(device, SDIO_COMMAND(CMD_SEND_CSD, SDIO_RESPONSE_LONG,
      (uint32_t)device->address << 16), 0, response);
  if (res != E_OK)
    return res;
  processCardSpecificData(device, response);

  return E_OK;
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
static enum result cardInit(void *object, const void *configBase)
{
  const struct SdCardConfig * const config = configBase;
  struct SdCard * const device = object;
  enum result res;

  device->address = 0;
  device->blockCount = 0;
  device->capacity = SDCARD_SDSC;
  device->interface = config->interface;
  device->position = 0;
  device->type = SDCARD_1_0;

  if ((res = initializeCard(device)) != E_OK)
    return res;

  return E_OK;
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

      //TODO Hardcoded sector size
      if ((position >> 9) >= (uint64_t)device->blockCount)
        return E_VALUE;

      //TODO Add alignment check
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
  uint32_t mode;
  enum result res;

  if (!blocks)
    return 0;

  if ((res = ifGet(device->interface, IF_SDIO_MODE, &mode)) != E_OK)
    return res;

  const enum sdioCommand commandType = blocks == 1 ? CMD_READ_SINGLE_BLOCK
      : CMD_READ_MULTIPLE_BLOCK;
  const enum sdioResponse responseType = mode == SDIO_SPI ? SDIO_RESPONSE_NONE
      : SDIO_RESPONSE_SHORT;

  const uint32_t command = SDIO_COMMAND(commandType, responseType,
      SDIO_DATA_MODE | SDIO_AUTO_STOP);
  const uint32_t argument = device->capacity == SDCARD_SDSC ?
      (uint32_t)device->position : (uint32_t)(device->position >> BLOCK_POW);

  if ((res = ifSet(device->interface, IF_SDIO_COMMAND, &command)) != E_OK)
    return res;
  if ((res = ifSet(device->interface, IF_SDIO_ARGUMENT, &argument)) != E_OK)
    return res;

  if (!ifRead(device->interface, buffer, length))
    return E_ERROR;

  while ((res = ifGet(device->interface, IF_STATUS, 0)) == E_BUSY);

  return res == E_OK ? length : 0;
}
/*----------------------------------------------------------------------------*/
static uint32_t cardWrite(void *object, const uint8_t *buffer, uint32_t length)
{
  struct SdCard * const device = object;
}
