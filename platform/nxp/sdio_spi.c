/*
 * sdio_spi.c
 * Copyright (C) 2014 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <assert.h>
#include <bits.h>
#include <delay.h>
#include <memory.h>
#include <platform/nxp/sdio_spi.h>
/*----------------------------------------------------------------------------*/
#define COMMAND_CODE_MASK               BIT_FIELD(MASK(6), 0)
#define COMMAND_CODE(value)             BIT_FIELD((value), 0)
#define COMMAND_CODE_VALUE(command) \
    FIELD_VALUE((value), COMMAND_CODE_MASK, 0)
#define COMMAND_DATA_MASK               BIT_FIELD(MASK(2), 6)
#define COMMAND_DATA(value)             BIT_FIELD((value), 6)
#define COMMAND_DATA_VALUE(command) \
    FIELD_VALUE((value), COMMAND_DATA_MASK, 6)
#define COMMAND_RESPONSE_MASK           BIT_FIELD(MASK(2), 8)
#define COMMAND_RESPONSE(value)         BIT_FIELD((value), 8)
#define COMMAND_RESPONSE_VALUE(command) \
    FIELD_VALUE((value), COMMAND_RESPONSE_MASK, 8)
#define COMMAND_FLAGS_MASK              BIT_FIELD(MASK(8), 10)
#define COMMAND_FLAGS(value)            BIT_FIELD((value), 10)
#define COMMAND_FLAGS_VALUE(command) \
    FIELD_VALUE((value), COMMAND_FLAGS_MASK, 10)
/*----------------------------------------------------------------------------*/
static enum result execute(struct SdioSpi *);
/*----------------------------------------------------------------------------*/
static enum result sdioInit(void *, const void *);
static void sdioDeinit(void *);
static enum result sdioCallback(void *, void (*)(void *), void *);
static enum result sdioGet(void *, enum ifOption, void *);
static enum result sdioSet(void *, enum ifOption, const void *);
static uint32_t sdioRead(void *, uint8_t *, uint32_t);
static uint32_t sdioWrite(void *, const uint8_t *, uint32_t);
/*----------------------------------------------------------------------------*/
static const struct InterfaceClass sdioTable = {
    .size = sizeof(struct SdioSpi),
    .init = sdioInit,
    .deinit = sdioDeinit,

    .callback = sdioCallback,
    .get = sdioGet,
    .set = sdioSet,
    .read = sdioRead,
    .write = sdioWrite
};
/*----------------------------------------------------------------------------*/
const struct InterfaceClass * const SdioSpi = &sdioTable;
/*----------------------------------------------------------------------------*/
static enum result execute(struct SdioSpi *interface)
{
  const uint8_t flags = COMMAND_FLAGS_VALUE(interface->command);
  const enum sdioDataMode mode = COMMAND_DATA_VALUE(device->command);
  const enum sdioResponseType response =
      COMMAND_RESPONSE_VALUE(device->command);
}
/*----------------------------------------------------------------------------*/
uint32_t sdioPrepareCommand(uint8_t command, enum sdioDataMode dataMode,
    enum sdioResponseType responseType, uint8_t flags)
{
  return COMMAND_CODE(command) | COMMAND_DATA((uint8_t)dataMode)
      | COMMAND_RESPONSE((uint8_t)responseType) | COMMAND_FLAGS(flags);
}
/*----------------------------------------------------------------------------*/
static enum result sdioInit(void *object, const void *configBase)
{
  const struct SdioSpiConfig * const config = configBase;
  struct SdioSpi * const interface = object;
  enum result res;

  interface->argument = 0;
  interface->command = 0;
  interface->interface = config->interface;
  memset(interface->response, 0, sizeof(interface->response));

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static void sdioDeinit(void *object __attribute__((unused)))
{

}
/*----------------------------------------------------------------------------*/
static enum result sdioCallback(void *object __attribute__((unused)),
    void (*callback)(void *) __attribute__((unused)),
    void *argument __attribute__((unused)))
{
  return E_ERROR;
}
/*----------------------------------------------------------------------------*/
static enum result sdioGet(void *object, enum ifOption option, void *data)
{
  struct SdioSpi * const interface = object;

  /* Additional options */
  switch ((enum sdioOption)option)
  {
    case IF_SDIO_RESPONSE:
    {
      const enum sdioResponseType response =
          COMMAND_RESPONSE_VALUE(device->command);

      if (response == SDIO_RESPONSE_NONE)
        return E_ERROR;

      if (response == SDIO_RESPONSE_LONG)
        memcpy(data, interface->response, sizeof(interface->response));
      else
        *(uint32_t *)data = interface->response[0];
      break;
    }

    default:
      break;
  }

  switch (option)
  {
    default:
      return E_ERROR;
  }
}
/*----------------------------------------------------------------------------*/
static enum result sdioSet(void *object, enum ifOption option,
    const void *data)
{
  struct SdioSpi * const interface = object;

  /* Additional options */
  switch ((enum sdioOption)option)
  {
    case IF_SDIO_EXECUTE:

      break;

    case IF_SDIO_ARGUMENT:
      interface->argument = *(const uint32_t *)data;
      break;

    case IF_SDIO_COMMAND:
      interface->command = *(const uint32_t *)data;
      break;

    default:
      break;
  }

  switch (option)
  {
    default:
      return E_ERROR;
  }
}
/*----------------------------------------------------------------------------*/
static uint32_t sdioRead(void *object, uint8_t *buffer, uint32_t length)
{
  struct SdioSpi * const interface = object;

}
/*----------------------------------------------------------------------------*/
static uint32_t sdioWrite(void *object, const uint8_t *buffer, uint32_t length)
{
  struct SdioSpi * const interface = object;

}
