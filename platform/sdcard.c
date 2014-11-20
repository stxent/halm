/*
 * sdcard.c
 * Copyright (C) 2014 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <assert.h>
#include <bits.h>
#include <delay.h>
#include <memory.h>
#include <platform/sdcard.h>
#include <platform/nxp/sdio_spi.h> // FIXME Rewrite
/*----------------------------------------------------------------------------*/
#define BLOCK_POW       9
#define TOKEN_DATA_MASK 0x1F
/*----------------------------------------------------------------------------*/
#define OCR_CCS         BIT(30) /* Card Capacity Status */
#define OCR_HCS         BIT(30) /* Host Capacity Support */
/*----------------------------------------------------------------------------*/
enum sdioCommand
{
  CMD_GO_IDLE_STATE     = 0,
  CMD_SEND_IF_COND      = 8,
  CMD_STOP_TRANSMISSION = 12,
  CMD_READ              = 17,
  CMD_READ_MULTIPLE     = 18,
  CMD_WRITE             = 24,
  CMD_WRITE_MULTIPLE    = 25,
  CMD_APP_CMD           = 55,
  CMD_READ_OCR          = 58,
  ACMD_SD_SEND_OP_COND  = 41
};
/*----------------------------------------------------------------------------*/
/* Direct operations with token variables are correct only on LE machines */
enum sdioToken
{
  TOKEN_DATA_ACCEPTED     = 0x05,
  TOKEN_DATA_CRC_ERROR    = 0x0B,
  TOKEN_DATA_WRITE_ERROR  = 0x0D,
  TOKEN_START             = 0xFE,
  TOKEN_START_MULTIPLE    = 0xFC,
  TOKEN_STOP              = 0xFD
};
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
static enum result resetCard(struct SdCard *device)
{
  uint32_t argument, command, response;
  enum result res;

  /* Send reset command */
  command = sdioPrepareCommand(CMD_GO_IDLE_STATE, SDIO_DATA_NONE,
      SDIO_RESPONSE_NONE, SDIO_INITIALIZE);
  argument = 0;

  ifSet(device->interface, IF_SDIO_COMMAND, &command);
  ifSet(device->interface, IF_SDIO_ARGUMENT, &argument);
  ifSet(device->interface, IF_SDIO_EXECUTE, 0);

  while ((res = ifGet(device->interface, IF_STATUS, 0)) == E_BUSY);
  if (res != E_OK)
    return res;

  /* Detect card version */
  command = sdioPrepareCommand(CMD_SEND_IF_COND, SDIO_DATA_NONE,
      SDIO_RESPONSE_SHORT, 0);
  argument = 0x000001AA;

  ifSet(device->interface, IF_SDIO_COMMAND, &command);
  ifSet(device->interface, IF_SDIO_ARGUMENT, &argument);
  ifSet(device->interface, IF_SDIO_EXECUTE, 0);

  while (ifGet(device->interface, IF_STATUS, 0) == E_BUSY);
  if (res != E_OK)
    return res;

  ifGet(device->interface, IF_SDIO_RESPONSE, &response);

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static enum result cardInit(void *object, const void *configBase)
{
  const struct SdCardConfig * const config = configBase;
  struct SdCard * const device = object;
  enum result res;

  device->interface = config->interface;

  if ((res = resetCard(device)) != E_OK)
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
}
/*----------------------------------------------------------------------------*/
static enum result cardSet(void *object, enum ifOption option,
    const void *data)
{
  struct SdCard * const device = object;
}
/*----------------------------------------------------------------------------*/
static uint32_t cardRead(void *object, uint8_t *buffer, uint32_t length)
{
  struct SdCard * const device = object;
}
/*----------------------------------------------------------------------------*/
static uint32_t cardWrite(void *object, const uint8_t *buffer, uint32_t length)
{
  struct SdCard * const device = object;
}
