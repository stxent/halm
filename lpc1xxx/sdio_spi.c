/*
 * sdio_spi.c
 * Copyright (C) 2013 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <assert.h>
#include "sdio_spi.h"
#include "ssp.h" //XXX
/*----------------------------------------------------------------------------*/
enum sdioCommands
{
//  CMD_GOIDLESTATE     = 0,
//  CMD_SENDOPCOND      = 1,
//  CMD_READCSD         = 9,
//  CMD_READCID         = 10,
//  CMD_SENDSTATUS      = 13,
//  CMD_READSINGLEBLOCK = 17,
//  CMD_WRITE           = 24,
//  CMD_WRITE_MULTIPLE  = 25,
  CMD0 = 0, /* GO_IDLE_STATE */
  CMD8 = 8, /* Send interface condition SEND_IF_COND */
  CMD13 = 13, /* SEND_STATUS */
  CMD55 = 55, /* Application command APP_CMD */
  CMD58 = 58, /* Read OCR READ_OCR */
  ACMD41 = 41, /* Send host capacity support information SD_SEND_OP_COND */
};
/*----------------------------------------------------------------------------*/
/*
0x40: Argument out of bounds
0x20: Address out of bounds
0x10: Error during erase sequence
0x08: CRC failed
0x04: Illegal command
0x02: Erase reset (see SanDisk docs p5-13)
0x01: Card is initializing
0x0001: Card is Locked
0x0002: WP Erase Skip, Lock/Unlock Cmd Failed
0x0004: General / Unknown error - card broken?
0x0008: Internal card controller error
0x0010: Card internal ECC was applied, but failed to correct the data
0x0020: Write protect violation
0x0040: An invalid selection, sectors for erase
0x0080: Out of Range, CSD_Overwrite
*/

//enum sdioResponse
//{
//  SDIO_OK = 0,
//  SDIO_ERROR,
//  SDIO_INITIALIZING,
//};
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
const uint8_t tableCrc7[256] = {
    0x00, 0x09, 0x12, 0x1B, 0x24, 0x2D, 0x36, 0x3F,
    0x48, 0x41, 0x5A, 0x53, 0x6C, 0x65, 0x7E, 0x77,
    0x19, 0x10, 0x0B, 0x02, 0x3D, 0x34, 0x2F, 0x26,
    0x51, 0x58, 0x43, 0x4A, 0x75, 0x7C, 0x67, 0x6E,
    0x32, 0x3B, 0x20, 0x29, 0x16, 0x1F, 0x04, 0x0D,
    0x7A, 0x73, 0x68, 0x61, 0x5E, 0x57, 0x4C, 0x45,
    0x2B, 0x22, 0x39, 0x30, 0x0F, 0x06, 0x1D, 0x14,
    0x63, 0x6A, 0x71, 0x78, 0x47, 0x4E, 0x55, 0x5C,
    0x64, 0x6D, 0x76, 0x7F, 0x40, 0x49, 0x52, 0x5B,
    0x2C, 0x25, 0x3E, 0x37, 0x08, 0x01, 0x1A, 0x13,
    0x7D, 0x74, 0x6F, 0x66, 0x59, 0x50, 0x4B, 0x42,
    0x35, 0x3C, 0x27, 0x2E, 0x11, 0x18, 0x03, 0x0A,
    0x56, 0x5F, 0x44, 0x4D, 0x72, 0x7B, 0x60, 0x69,
    0x1E, 0x17, 0x0C, 0x05, 0x3A, 0x33, 0x28, 0x21,
    0x4F, 0x46, 0x5D, 0x54, 0x6B, 0x62, 0x79, 0x70,
    0x07, 0x0E, 0x15, 0x1C, 0x23, 0x2A, 0x31, 0x38,
    0x41, 0x48, 0x53, 0x5A, 0x65, 0x6C, 0x77, 0x7E,
    0x09, 0x00, 0x1B, 0x12, 0x2D, 0x24, 0x3F, 0x36,
    0x58, 0x51, 0x4A, 0x43, 0x7C, 0x75, 0x6E, 0x67,
    0x10, 0x19, 0x02, 0x0B, 0x34, 0x3D, 0x26, 0x2F,
    0x73, 0x7A, 0x61, 0x68, 0x57, 0x5E, 0x45, 0x4C,
    0x3B, 0x32, 0x29, 0x20, 0x1F, 0x16, 0x0D, 0x04,
    0x6A, 0x63, 0x78, 0x71, 0x4E, 0x47, 0x5C, 0x55,
    0x22, 0x2B, 0x30, 0x39, 0x06, 0x0F, 0x14, 0x1D,
    0x25, 0x2C, 0x37, 0x3E, 0x01, 0x08, 0x13, 0x1A,
    0x6D, 0x64, 0x7F, 0x76, 0x49, 0x40, 0x5B, 0x52,
    0x3C, 0x35, 0x2E, 0x27, 0x18, 0x11, 0x0A, 0x03,
    0x74, 0x7D, 0x66, 0x6F, 0x50, 0x59, 0x42, 0x4B,
    0x17, 0x1E, 0x05, 0x0C, 0x33, 0x3A, 0x21, 0x28,
    0x5F, 0x56, 0x4D, 0x44, 0x7B, 0x72, 0x69, 0x60,
    0x0E, 0x07, 0x1C, 0x15, 0x2A, 0x23, 0x38, 0x31,
    0x46, 0x4F, 0x54, 0x5D, 0x62, 0x6B, 0x70, 0x79
};
/*----------------------------------------------------------------------------*/
uint8_t calculateCrc7(uint8_t *buffer, uint8_t length)
{
  uint8_t crc = 0x00;

  while (length--)
    crc = tableCrc7[(crc << 1) ^ *buffer++];
  return crc;
}
/*----------------------------------------------------------------------------*/
static enum result sendCommand(struct SdioSpi *device,
    enum sdioCommands command, uint32_t parameter)
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
  /* Checksum should be valid only for first command */
  if (command == CMD0 || command == CMD8)
    buffer[6] = calculateCrc7(buffer + 1, 5) << 1;
  else
    buffer[6] = 0x00;
  buffer[6] |= 0x01; /* Add end bit */
  buffer[7] = 0xFF;

  gpioWrite(&device->csPin, 0);
  if (ifWrite(device->interface, buffer, sizeof(buffer)) != sizeof(buffer))
    res = E_INTERFACE; /* Interface error */
  gpioWrite(&device->csPin, 1);
  return res;
}
/*----------------------------------------------------------------------------*/
static enum result getFirstByte(struct SdioSpi *device)
{
  uint8_t counter, value = 0xFF;

  /* Response will come after 1..8 queries */
  for (counter = 0; counter < 8 && value == 0xFF; counter++)
  {
    if (!ifRead(device->interface, &value, 1))
      return E_INTERFACE; /* Interface error */
  }
  switch (value)
  {
    case 0x00:
      return E_OK;
    case 0x01:
      return E_BUSY;
//    case 0x04:
//      return E_INVALID;
    case 0x20:
      return E_FAULT;
    default:
      return E_DEVICE;
  }
}
/*----------------------------------------------------------------------------*/
static enum result getResponseR1(struct SdioSpi *device)
{
  enum result res;

  gpioWrite(&device->csPin, 0);
  res = getFirstByte(device);
  gpioWrite(&device->csPin, 1);
  return res;
}
/*----------------------------------------------------------------------------*/
/* TODO Optimize */
/* Response to SEND_STATUS command */
static enum result getResponseR2(struct SdioSpi *device/*, uint8_t *state*/)
{
  uint8_t state;
  enum result res;

  gpioWrite(&device->csPin, 0);
  res = getFirstByte(device);
  if (res != E_DEVICE && res != E_INTERFACE)
  {
    if (!ifRead(device->interface, &state, 1))
      res = E_INTERFACE;
    res = !state ? E_OK : E_ERROR;
  }
  gpioWrite(&device->csPin, 1);
  return res;
}
/*----------------------------------------------------------------------------*/
/* Response to READ_OCR command */
static enum result getResponseR3(struct SdioSpi *device, uint32_t *ocr)
{
  uint8_t buffer[4];
  enum result res;

  gpioWrite(&device->csPin, 0);
  res = getFirstByte(device);
  if (res != E_DEVICE && res != E_INTERFACE)
  {
    /* Response comes in big-endian format */
    if (ifRead(device->interface, buffer, sizeof(buffer)) != sizeof(buffer))
      res = E_INTERFACE;
  }
  gpioWrite(&device->csPin, 1);
  *ocr = ((uint32_t)buffer[0] << 24) | ((uint32_t)buffer[1] << 16)
    | ((uint32_t)buffer[2] << 8) | (uint32_t)buffer[3];
  return res;
}
/*----------------------------------------------------------------------------*/
/* Response to SEND_IF_COND command */
static enum result getResponseR7(struct SdioSpi *device, uint32_t *condition)
{
  uint8_t buffer[4];
  enum result res;

  gpioWrite(&device->csPin, 0);
  res = getFirstByte(device);
  if (res != E_DEVICE && res != E_INTERFACE)
  {
    /* Response comes in big-endian format */
    if (ifRead(device->interface, buffer, sizeof(buffer)) != sizeof(buffer))
      res = E_INTERFACE;
  }
  gpioWrite(&device->csPin, 1);
  *condition = ((uint32_t)buffer[0] << 24) | ((uint32_t)buffer[1] << 16)
    | ((uint32_t)buffer[2] << 8) | (uint32_t)buffer[3];
  return res;
}
/*----------------------------------------------------------------------------*/
static enum result sdioInit(void *object, const void *configPtr)
{
  /* Set pointer to device configuration data */
  const struct SdioSpiConfig *config = configPtr;
  const uint32_t lowSpeed = 200000;
  struct SdioSpi *device = object;
  uint32_t srcSpeed, response;
  uint16_t counter;
  struct Ssp *fix = (struct Ssp *)config->interface; //XXX
  enum result res;

  /* Check device configuration data */
  assert(config);
  assert(config->interface);

//  device->ready = false;
  device->highCapacity = false;
  device->interface = config->interface;

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
    if ((res = getResponseR1(device)) == E_BUSY)
      break;
  }

  if (res != E_BUSY)
    return res; //FIXME

  sendCommand(device, CMD8, 0x000001AA);
  /*if ((*/res = getResponseR7(device, &response);/*) != E_OK)*/
//    return res; //FIXME
  //FIXME check pattern

  /* Wait till card is ready */
  for (counter = 0; counter < 32768; counter++) //XXX
  {
    sendCommand(device, CMD55, 0);
    /*if ((*/res = getResponseR1(device);/*) != E_OK)
      break;*/
    sendCommand(device, ACMD41, (1 << 30)); //FIXME Add define HCS
    if ((res = getResponseR1(device)) == E_OK)
      break;
  }

  if (res != E_OK)
    return res;
  sendCommand(device, CMD58, 0);
  if ((res = getResponseR7(device, &response)) != E_OK)
    return res; //FIXME
  if (response & (1 << 30)) //FIXME Add define CCS
    device->highCapacity = true;

  /* Increase speed after initialization */
  ifSet(device->interface, IF_RATE, &srcSpeed);

  sendCommand(device, CMD13, 0);
  if ((res = getResponseR2(device)) != E_OK)
    return res;

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

//  uint8_t cardresp;
//  uint8_t firstblock;
//  uint16_t fb_timeout = 0xffff;
//  uint32_t place;
//
//  place = 512 * dwAddress;
//  Command(CMD_READSINGLEBLOCK, place);
//
//  cardresp = Resp8b();    /* Card response */
//
//  /* Wait for startblock */
//  do {
//    firstblock = Resp8b();
//  } while (firstblock == 0xff && fb_timeout--);
//
//  if (cardresp != 0x00 || firstblock != 0xfe) {
//    Resp8bError(firstblock);
//    return -1;
//  }
//
//  SPIRecvN(pbBuf, 512);
//
//  /* Checksum (2 byte) - ignore for now */
//  SPISend(0xff);
//  SPISend(0xff);
//
//  return 0;

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

      return E_OK;
    default:
      return E_ERROR;
  }
}
