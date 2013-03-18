/*
 * sdio_spi.c
 * Copyright (C) 2013 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <assert.h>
#include "sdio_spi.h"
#include "ssp.h" //XXX
/*----------------------------------------------------------------------------*/
enum sdioCommand
{
  CMD0 = 0, /* GO_IDLE_STATE */
  CMD8 = 8, /* Send interface condition SEND_IF_COND */
  CMD13 = 13, /* SEND_STATUS */
  CMD55 = 55, /* Application command APP_CMD */
  CMD58 = 58, /* Read OCR READ_OCR */
  ACMD41 = 41, /* Send host capacity support information SD_SEND_OP_COND */
  CMD_READ            = 17,
  CMD_READ_MULTIPLE   = 18,
  CMD_WRITE           = 24,
  CMD_WRITE_MULTIPLE  = 25,
};
/*----------------------------------------------------------------------------*/
enum sdioResponse
{
  SDIO_INIT             = 0x0001,
  SDIO_ERASE_RESET      = 0x0002,
  SDIO_ILLEGAL_COMMAND  = 0x0004,
  SDIO_CRC_ERROR        = 0x0008,
  SDIO_ERASE_ERROR      = 0x0010,
  SDIO_BAD_ADDRESS      = 0x0020,
  SDIO_BAD_ARGUMENT     = 0x0040,
//  SDIO_CARD_LOCKED        = 0x0100,
//  SDIO_WP_ERASE_SKIP      = 0x0200,
//  SDIO_ERROR              = 0x0400,
//  SDIO_INTERNAL_ERROR     = 0x0800,
//  SDIO_ECC_ERROR          = 0x1000,
//  SDIO_WRITE_PROTECT      = 0x2000,
//  SDIO_INVALID_SELECTION  = 0x4000,
//  SDIO_OUT_OF_RANGE       = 0x8000
};
/*----------------------------------------------------------------------------*/
struct ShortResponse
{
    uint8_t value;
};
/*----------------------------------------------------------------------------*/
struct LongResponse
{
    uint32_t payload;
    uint8_t value;
};
/*----------------------------------------------------------------------------*/
static enum result sdioInit(void *, const void *);
static void sdioDeinit(void *);
static uint32_t sdioRead(void *, uint8_t *, uint32_t);
static uint32_t sdioWrite(void *, const uint8_t *, uint32_t);
static enum result sdioGet(void *, enum ifOption, void *);
static enum result sdioSet(void *, enum ifOption, const void *);
/*----------------------------------------------------------------------------*/
static const struct InterfaceClass sdioTable = {
    .size = sizeof(struct SdioSpi),
    .init = sdioInit,
    .deinit = sdioDeinit,

    .read = sdioRead,
    .write = sdioWrite,
    .get = sdioGet,
    .set = sdioSet
};
/*----------------------------------------------------------------------------*/
const struct InterfaceClass *SdioSpi = &sdioTable;
static const uint8_t dummyByte = 0xFF; /* Emulate high level during setup */
/*----------------------------------------------------------------------------*/
static enum result sendCommand(struct SdioSpi *device, enum sdioCommand command,
    uint32_t parameter)
{
  uint8_t buffer[8];
  enum result res = E_OK;

  /* Fill buffer */
  buffer[0] = 0xFF;
  buffer[1] = 0x40 | (uint8_t)command;
  buffer[2] = (uint8_t)(parameter >> 24);
  buffer[3] = (uint8_t)(parameter >> 16);
  buffer[4] = (uint8_t)(parameter >> 8);
  buffer[5] = (uint8_t)(parameter);
  /* Checksum should be valid only for first CMD0 and CMD8 commands */
  switch (command)
  {
    case CMD0:
      buffer[6] = 0x94;
      break;
    case CMD8:
      buffer[6] = 0x86;
      break;
    default:
      buffer[6] = 0x00;
      break;
  }
  buffer[6] |= 0x01; /* Add end bit */
  buffer[7] = 0xFF;

  gpioWrite(&device->csPin, 0);
  if (ifWrite(device->interface, buffer, sizeof(buffer)) != sizeof(buffer))
    res = E_INTERFACE; /* Interface error */
  gpioWrite(&device->csPin, 1);
  return res;
}
/*----------------------------------------------------------------------------*/
static enum result waitForData(struct SdioSpi *device, uint8_t *value)
{
  uint8_t counter;

  /* Response will come after 1..8 queries */
  for (counter = 8; counter; counter--)
  {
    if (!ifRead(device->interface, value, 1))
      return E_INTERFACE; /* Interface error */
    if (*value != 0xFF)
      break;
  }
  return !counter ? E_DEVICE : E_OK;
}
/*----------------------------------------------------------------------------*/
static enum result getShortResponse(struct SdioSpi *device,
    struct ShortResponse *response)
{
  uint8_t value;
  enum result res;

  gpioWrite(&device->csPin, 0);
  res = waitForData(device, &value);
  gpioWrite(&device->csPin, 1);
  if (res == E_OK)
    response->value = value;
  return res;
}
/*----------------------------------------------------------------------------*/
/* Responses to READ_OCR and SEND_IF_COND commands */
static enum result getLongResponse(struct SdioSpi *device,
    struct LongResponse *response)
{
  uint8_t buffer[4];
  enum result res;

  gpioWrite(&device->csPin, 0);
  res = waitForData(device, buffer);
  if (res == E_OK)
  {
    response->value = *buffer;
    if (ifRead(device->interface, buffer, sizeof(buffer)) != sizeof(buffer))
      res = E_INTERFACE;
    /* Response comes in big-endian format */
    response->payload = (uint32_t)buffer[3] | ((uint32_t)buffer[2] << 8)
        | ((uint32_t)buffer[1] << 16) | ((uint32_t)buffer[0] << 24);
  }
  gpioWrite(&device->csPin, 1);
  return res;
}
/*----------------------------------------------------------------------------*/
static enum result sdioInit(void *object, const void *configPtr)
{
  /* Set pointer to device configuration data */
  const struct SdioSpiConfig *config = configPtr;
  const uint32_t lowSpeed = 200000;
  struct SdioSpi *device = object;
  uint32_t srcSpeed;
  uint16_t counter;
  struct ShortResponse shortResp;
  struct LongResponse longResp;
  bool oldCard = true;
  enum result res;

  struct Ssp *fix = (struct Ssp *)config->interface; //XXX

  /* Check device configuration data */
  assert(config);
  assert(config->interface);

//  device->ready = false;
  device->highCapacity = false;
  device->interface = config->interface;
  device->position = 0;

  gpioSetPull(&fix->mosiPin, GPIO_PULLUP); //XXX
  gpioSetPull(&fix->misoPin, GPIO_PULLUP); //XXX

  device->csPin = gpioInit(config->cs, GPIO_OUTPUT);
  gpioWrite(&device->csPin, 1);

  ifGet(device->interface, IF_RATE, &srcSpeed);
  ifSet(device->interface, IF_RATE, &lowSpeed);

  for (counter = 0; counter < 10; counter++)
    ifWrite(device->interface, &dummyByte, 1);

  /* Try to send reset command up to 64 times */
  for (counter = 0; counter < 64; counter++) //XXX
  {
    sendCommand(device, CMD0, 0);
    if ((res = getShortResponse(device, &shortResp)) != E_OK)
      return res; //FIXME
    if (shortResp.value & SDIO_INIT)
      break;
  }
  if (!(shortResp.value & SDIO_INIT))
    return E_ERROR; //FIXME

  sendCommand(device, CMD8, 0x000001AA); /* TODO Add define */
  if ((res = getLongResponse(device, &longResp)) != E_OK)
    return res; //FIXME
  if (!(longResp.value & SDIO_ILLEGAL_COMMAND))
  {
    if (longResp.payload != 0x000001AA)
      return E_ERROR; /* Pattern mismatched */
    oldCard = false;
  }

  /* Wait till card is ready */
  for (counter = 0; counter < 32768; counter++) //XXX
  {
    sendCommand(device, CMD55, 0);
    if ((res = getShortResponse(device, &shortResp)) != E_OK)
      return res; //FIXME
    sendCommand(device, ACMD41, (1 << 30)); //FIXME Add define HCS
    if ((res = getShortResponse(device, &shortResp)) != E_OK)
      return res;
    if (!shortResp.value)
      break;
  }
  if (shortResp.value)
    return E_ERROR; //FIXME

  if (!oldCard) /* SD Card version 2.0 or later */
  {
    sendCommand(device, CMD58, 0);
    if ((res = getLongResponse(device, &longResp)) != E_OK)
      return res; //FIXME
    if (longResp.payload & (1 << 30)) //FIXME Add define CCS
      device->highCapacity = true;
  }

  /* Increase speed after initialization */
  ifSet(device->interface, IF_RATE, &srcSpeed);

  device->lock = MUTEX_UNLOCKED;
//  device->ready = true;

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static void sdioDeinit(void *object)
{
  struct SdioSpi *device = object;

  gpioDeinit(&device->csPin);
}
/*----------------------------------------------------------------------------*/
static uint32_t sdioRead(void *object, uint8_t *buffer, uint32_t length)
{
  struct SdioSpi *device = object;

  uint32_t address, count;
  uint16_t counter;
  uint8_t crcBuffer[2];
  struct ShortResponse shortResp;
  enum result res;

  /* TODO Check upper limit */
  if (length & ((1 << 9) - 1)) /* Unsupported block length */
    return 0;
  address = device->highCapacity ? (uint32_t)(device->position >> 9)
      : (uint32_t)device->position;
  sendCommand(device, CMD_READ, address);
  if ((res = getShortResponse(device, &shortResp)) != E_OK)
    return 0;

  /* Wait for data token */
  for (counter = 0xFFFF; counter; counter--)
  {
    res = getShortResponse(device, &shortResp);
    if (res != E_OK && res != E_DEVICE)
      return 0;
    if (res == E_OK && shortResp.value == 0xFE) /* TODO Add define */
      break;
  }
  if (!counter)
    return 0;

  gpioWrite(&device->csPin, 0);
  count = ifRead(device->interface, buffer, length);
  if (count != length)
  {
    gpioWrite(&device->csPin, 1);
    return 0;
  }
  count = ifRead(device->interface, crcBuffer, sizeof(crcBuffer));
  gpioWrite(&device->csPin, 1);
  if (count != sizeof(crcBuffer))
    return 0;

  return length;
}
/*----------------------------------------------------------------------------*/
static uint32_t sdioWrite(void *object, const uint8_t *buffer, uint32_t length)
{
  struct SdioSpi *device = object;

//  uint32_t place;
//  uint16_t t = 0;
//
//  place = 512 * dwAddress;
//  Command(CMD_WRITE, place);
//
//  Resp8b();       /* Card response */
//
//  SPISend(0xfe);      /* Start block */
//  SPISendN(pbBuf, 512);
//  SPISend(0xff);      /* Checksum part 1 */
//  SPISend(0xff);      /* Checksum part 2 */
//
//  SPISend(0xff);
//
//  while (SPISend(0xff) != 0xff) {
//    t++;
//  }
//
//  return 0;

  return length;
}
/*----------------------------------------------------------------------------*/
static enum result sdioGet(void *object, enum ifOption option, void *data)
{
  struct SdioSpi *device = object;

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
static enum result sdioSet(void *object, enum ifOption option,
    const void *data)
{
  struct SdioSpi *device = object;

  switch (option)
  {
    case IF_ADDRESS:
      /* TODO Add boundary check */
      device->position = *(uint64_t *)data;// + 8192 * 512; //XXX
      return E_OK;
    default:
      return E_ERROR;
  }
}
