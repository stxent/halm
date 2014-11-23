/*
 * sdio_spi.c
 * Copyright (C) 2014 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <assert.h>
#include <string.h>
#include <bits.h>
#include <delay.h>
#include <memory.h>
#include <modules/sdio.h>
#include <modules/sdio_defs.h>
#include <platform/nxp/sdio_spi.h>
/*----------------------------------------------------------------------------*/
enum sdioResponseFlags
{
  FLAG_IDLE_STATE       = 0x01,
  FLAG_ERASE_RESET      = 0x02,
  FLAG_ILLEGAL_COMMAND  = 0x04,
  FLAG_CRC_ERROR        = 0x08,
  FLAG_ERASE_ERROR      = 0x10,
  FLAG_BAD_ADDRESS      = 0x20,
  FLAG_BAD_PARAMETER    = 0x40,
  /* MSB in response tokens is always set to zero */
  FLAG_NO_RESPONSE      = 0x80
};
/*----------------------------------------------------------------------------*/
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
static void acquireBus(struct SdioSpi *);
static void execute(struct SdioSpi *);
static enum result getLongResponse(struct SdioSpi *, uint32_t *);
static enum result getShortResponse(struct SdioSpi *, uint32_t *);
static void releaseBus(struct SdioSpi *);
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
static void acquireBus(struct SdioSpi *interface)
{
  ifSet(interface->interface, IF_ACQUIRE, 0);
  pinReset(interface->cs);
}
/*----------------------------------------------------------------------------*/
static void execute(struct SdioSpi *interface)
{
  const uint32_t argument = toBigEndian32(interface->argument);
  const uint8_t code = COMMAND_CODE_VALUE(interface->command);
  const uint8_t flags = COMMAND_FLAG_VALUE(interface->command);
  const enum sdioResponse response = COMMAND_RESP_VALUE(interface->command);
  uint32_t bytesWritten;
  enum result res;

  interface->status = E_BUSY;

  if (flags & SDIO_INITIALIZE)
  {
    /* Send initialization sequence */
    memset(interface->buffer, 0xFF, 10);

    ifSet(interface->interface, IF_ACQUIRE, 0);
    bytesWritten = ifWrite(interface->interface, interface->buffer, 10);

    // TODO Rewrite
    while (ifGet(interface->interface, IF_STATUS, 0) == E_BUSY);
    ifSet(interface->interface, IF_RELEASE, 0);

    if (bytesWritten != 10)
    {
      interface->status = E_INTERFACE;
      return;
    }
  }

  /* Fill the buffer */
  interface->buffer[0] = 0xFF;
  interface->buffer[1] = 0x40 | code;
  memcpy(interface->buffer + 2, &argument, sizeof(argument));

  // TODO Remove hardcoded values
  /* Checksum should be valid only for first CMD0 and CMD8 commands */
  switch (code)
  {
    case 0:
      interface->buffer[6] = 0x94;
      break;

    case 8:
      interface->buffer[6] = 0x86;
      break;

    default:
      interface->buffer[6] = 0x00;
      break;
  }
  interface->buffer[6] |= 0x01; /* Add end bit */
  interface->buffer[7] = 0xFF;

  acquireBus(interface);

  interface->state = SDIO_SPI_STATE_SEND_CMD;
  bytesWritten = ifWrite(interface->interface, interface->buffer, 8);
  if (bytesWritten != 8)
  {
    interface->status = E_INTERFACE;
    interface->state = SDIO_SPI_STATE_IDLE;
    releaseBus(interface);
    return;
  }
  else
  {

  }

  while (interface->status == E_BUSY); //FIXME
}
/*----------------------------------------------------------------------------*/
static enum result getLongResponse(struct SdioSpi *interface, uint32_t *value)
{
  uint8_t token;
  enum result res;

  if ((res = waitForData(interface, &token)) == E_OK)
  {
    if ((res = processResponseToken(token)) != E_OK)
      return res;
  }
  else
  {
    return res;
  }

  if ((res = waitForData(interface, &token)) == E_OK)
  {
    if (token != TOKEN_START)
      return E_ERROR;
  }
  else
  {
    return res;
  }

  /* Read 16 bytes of the long response */
  if (ifRead(interface->interface, interface->buffer, 16) != 16)
    return E_INTERFACE;

  for (uint8_t index = 0; index < 4; ++index)
    value[3 - index] = fromBigEndian32(((uint32_t *)interface->buffer)[index]);

  /* Read 2 bytes of checksum */
  if (ifRead(interface->interface, interface->buffer, 2) != 2)
    return E_INTERFACE;
  //TODO Check CRC

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static enum result getShortResponse(struct SdioSpi *interface, uint32_t *value)
{
  uint8_t token;
  enum result res;

  if ((res = waitForData(interface, &token)) == E_OK)
  {
    if (ifRead(interface->interface, interface->buffer, 4) != 4)
      return E_INTERFACE;

    res = processResponseToken(token);

    /* Response comes in big-endian format */
    *value = fromBigEndian32(*(uint32_t *)interface->buffer);
  }

  return res;
}
/*----------------------------------------------------------------------------*/
static enum result checksumProcess(struct SdioSpi *interface, uint16_t checksum)
{
  //TODO
  return E_OK;
}
/*----------------------------------------------------------------------------*/
static enum result checksumRequest(struct SdioSpi *interface)
{
  if (ifRead(interface->interface, interface->buffer, 2) != 2)
    return E_INTERFACE;

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static enum result responseProcess(struct SdioSpi *interface)
{
  const enum sdioResponse response = COMMAND_RESP_VALUE(interface->command);

  /* Responses come in big-endian format */
  if (response == SDIO_RESPONSE_SHORT)
  {
    interface->response[0] = fromBigEndian32(*(uint32_t *)interface->buffer);
  }
  else
  {
    const uint32_t *buffer = (const uint32_t *)interface->buffer;

    for (uint8_t index = 0; index < 4; ++index)
      interface->response[3 - index] = fromBigEndian32(buffer[index]);
  }
  return E_OK;
}
/*----------------------------------------------------------------------------*/
static enum result responseRequest(struct SdioSpi *interface,
    const enum sdioResponse type)
{
  if (type == SDIO_RESPONSE_SHORT)
  {
    /* Read 32-bit response */
    if (ifRead(interface->interface, interface->buffer, 4) != 4)
      return E_INTERFACE;
  }
  else
  {
    /* Read 128-bit response */
    if (ifRead(interface->interface, interface->buffer, 16) != 16)
      return E_INTERFACE;
  }

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static enum result responseTokenProcess(struct SdioSpi *interface)
{
  const uint8_t token = interface->buffer[0];
  enum result res;

  if (token == 0xFF)
    res = interface->iteration >= 8 ? E_TIMEOUT : E_BUSY;
  else if (token & FLAG_NO_RESPONSE)
    res = E_DEVICE;
  else if (token & FLAG_ILLEGAL_COMMAND)
    res = E_INVALID;
  else if ((token & FLAG_IDLE_STATE) == FLAG_IDLE_STATE)
    res = E_IDLE;
  else if (token)
    res = E_ERROR;
  else
    res = E_OK;

  return res;
}
/*----------------------------------------------------------------------------*/
static enum result tokenProcess(struct SdioSpi *interface,
    enum sdioToken expected)
{
  const uint8_t token = interface->buffer[0];
  enum result res;

  if (token == 0xFF)
    res = interface->iteration >= 8 ? E_TIMEOUT : E_BUSY;
  else if (token != expected)
    res = E_DEVICE;
  else
    res = E_OK;

  return res;
}
/*----------------------------------------------------------------------------*/
static enum result tokenRequest(struct SdioSpi *interface)
{
  return ifRead(interface->interface, interface->buffer, 1) == 1 ? E_OK
      : E_INTERFACE;
}
/*----------------------------------------------------------------------------*/
static void interruptHandler(void *object)
{
  struct SdioSpi * const interface = object;
  enum result res;
  bool event = false;

  pinSet(interface->debug);
  switch (interface->state)
  {
    case SDIO_SPI_STATE_SEND_CMD:
      if ((res = tokenRequest(interface)) == E_OK)
      {
        interface->state = SDIO_SPI_STATE_WAIT_RESPONSE;
        interface->iteration = 0;
      }
      else
      {
        /* Error occurred */
        interface->status = res;
        interface->state = SDIO_SPI_STATE_IDLE;
        event = true;
      }
      break;

    case SDIO_SPI_STATE_WAIT_RESPONSE:
      res = responseTokenProcess(interface);
      if (res == E_BUSY)
      {
        /* Device is busy, try again */
        if ((res = tokenRequest(interface)) == E_OK)
        {
          ++interface->iteration;
        }
        else
        {
          /* Error occurred */
          interface->status = res;
          interface->state = SDIO_SPI_STATE_IDLE;
          event = true;
        }
      }
      else if (res == E_OK || res == E_IDLE)
      {
        const enum sdioResponse response =
            COMMAND_RESP_VALUE(interface->command);

        if (response == SDIO_RESPONSE_NONE)
        {
          /* No response required */
          interface->status = res;
          interface->state = SDIO_SPI_STATE_IDLE;
          event = true;
        }
        else if (response == SDIO_RESPONSE_SHORT)
        {
          interface->token = res;

          if ((res = responseRequest(interface, SDIO_RESPONSE_SHORT)) == E_OK)
          {
            interface->state = SDIO_SPI_STATE_READ_SHORT;
          }
          else
          {
            /* Error occurred */
            interface->status = res;
            interface->state = SDIO_SPI_STATE_IDLE;
            event = true;
          }
        }
        else
        {
          interface->token = res;

          //TODO Remove redundant code
          if ((res = tokenRequest(interface)) == E_OK)
          {
            interface->state = SDIO_SPI_STATE_WAIT_LONG;
            interface->iteration = 0;
          }
          else
          {
            /* Error occurred */
            interface->status = res;
            interface->state = SDIO_SPI_STATE_IDLE;
            event = true;
          }
        }
      }
      break;

    case SDIO_SPI_STATE_READ_SHORT:
      if ((res = responseProcess(interface)) != E_OK)
        interface->status = res;
      else
        interface->status = interface->token;
      interface->state = SDIO_SPI_STATE_IDLE;
      event = true;
      break;

    case SDIO_SPI_STATE_WAIT_LONG:
      res = tokenProcess(interface, TOKEN_START);
      if (res == E_BUSY)
      {
        /* Device is busy, try again */
        if ((res = tokenRequest(interface)) == E_OK)
        {
          ++interface->iteration;
        }
        else
        {
          /* Error occurred */
          interface->status = res;
          interface->state = SDIO_SPI_STATE_IDLE;
          event = true;
        }
      }
      else if (res == E_OK)
      {
        if ((res = responseRequest(interface, SDIO_RESPONSE_LONG)) == E_OK)
        {
          interface->state = SDIO_SPI_STATE_READ_LONG;
        }
        else
        {
          /* Error occurred */
          interface->status = res;
          interface->state = SDIO_SPI_STATE_IDLE;
          event = true;
        }
      }
      else
      {
        /* Error occurred */
        interface->status = res;
        interface->state = SDIO_SPI_STATE_IDLE;
        event = true;
      }
      break;

    case SDIO_SPI_STATE_READ_LONG:
      if ((res = responseProcess(interface)) == E_OK)
      {
        if ((res = checksumRequest(interface)) == E_OK)
        {
          interface->state = SDIO_SPI_STATE_READ_LONG_CRC;
        }
        else
        {
          /* Error occurred */
          interface->status = res;
          interface->state = SDIO_SPI_STATE_IDLE;
          event = true;
        }
      }
      else
      {
        /* Error occurred */
        interface->status = res;
        interface->state = SDIO_SPI_STATE_IDLE;
        event = true;
      }
      break;

    case SDIO_SPI_STATE_READ_LONG_CRC:
      interface->status = checksumProcess(interface, 0x0000); //TODO
      interface->state = SDIO_SPI_STATE_IDLE;
      event = true;

    default:
      break;
  }
  pinReset(interface->debug);

  if (event)
  {
    releaseBus(interface);

    if (interface->callback)
      interface->callback(interface->callbackArgument);
  }
}
/*----------------------------------------------------------------------------*/
static void releaseBus(struct SdioSpi *interface)
{
  pinSet(interface->cs);
  ifSet(interface->interface, IF_RELEASE, 0);
}
/*----------------------------------------------------------------------------*/
static enum result sdioInit(void *object, const void *configBase)
{
  const struct SdioSpiConfig * const config = configBase;
  struct SdioSpi * const interface = object;
  enum result res;

  interface->cs = pinInit(config->cs);
  if (!pinValid(interface->cs))
    return E_VALUE;
  pinOutput(interface->cs, 1);

  interface->interface = config->interface;

  res = ifSet(interface->interface, IF_ZEROCOPY, 0);
  if (res != E_OK)
    return res;

  res = ifCallback(interface->interface, interruptHandler, interface);
  if (res != E_OK)
    return res;

  //TODO Remove debug pin
  interface->debug = pinInit(PIN(1, 0));
  pinOutput(interface->debug, 0);

  interface->argument = 0;
  interface->callback = 0;
  interface->command = 0;
  interface->state = SDIO_SPI_STATE_IDLE;
  interface->status = E_OK;
  memset(interface->response, 0, sizeof(interface->response));

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static void sdioDeinit(void *object __attribute__((unused)))
{

}
/*----------------------------------------------------------------------------*/
static enum result sdioCallback(void *object, void (*callback)(void *),
    void *argument)
{
  struct SdioSpi * const interface = object;

  interface->callbackArgument = argument;
  interface->callback = callback;
  return E_OK;
}
/*----------------------------------------------------------------------------*/
static enum result sdioGet(void *object, enum ifOption option, void *data)
{
  struct SdioSpi * const interface = object;

  /* Additional options */
  switch ((enum sdioOption)option)
  {
    case IF_SDIO_MODE:
    {
      *(uint32_t *)data = SDIO_SPI;
      return E_OK;
    }

    case IF_SDIO_RESPONSE:
    {
      const enum sdioResponse response = COMMAND_RESP_VALUE(interface->command);

      if (response == SDIO_RESPONSE_NONE)
        return E_ERROR;

      if (response == SDIO_RESPONSE_SHORT)
        memcpy(data, interface->response, 4);
      else
        memcpy(data, interface->response, 16);
      return E_OK;
    }

    default:
      break;
  }

  switch (option)
  {
    case IF_STATUS:
      return interface->status;

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
      execute(interface);
      return E_OK;

    case IF_SDIO_ARGUMENT:
      interface->argument = *(const uint32_t *)data;
      return E_OK;

    case IF_SDIO_COMMAND:
      interface->command = *(const uint32_t *)data;
      return E_OK;

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
