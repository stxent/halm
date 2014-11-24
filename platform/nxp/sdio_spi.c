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
static void execute(struct SdioSpi *);
static void interruptHandler(void *);

static enum result checksumProcess(struct SdioSpi *, uint16_t);
static void checksumRequest(struct SdioSpi *);
static void responseProcess(struct SdioSpi *, enum sdioResponse);
static void responseRequest(struct SdioSpi *, enum sdioResponse);
static enum result responseTokenProcess(struct SdioSpi *);
static void sendCommand(struct SdioSpi *);
static void sendInitSequence(struct SdioSpi *);
static enum result tokenProcess(struct SdioSpi *, enum sdioToken);
static void tokenRequest(struct SdioSpi *);
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
static void execute(struct SdioSpi *interface)
{
  const uint16_t flags = COMMAND_FLAG_VALUE(interface->command);

  interface->status = E_BUSY;
  ifSet(interface->interface, IF_ACQUIRE, 0);
  pinSet(interface->debug2); //TODO Remove

  if (flags & SDIO_INITIALIZE)
  {
    interface->state = SDIO_SPI_STATE_INIT;
    sendInitSequence(interface);
  }
  else
  {
    interface->state = SDIO_SPI_STATE_SEND_CMD;
    sendCommand(interface);
  }

  while (interface->status == E_BUSY); //FIXME
}
/*----------------------------------------------------------------------------*/
static enum result checksumProcess(struct SdioSpi *interface, uint16_t checksum)
{
  //TODO
  return E_OK;
}
/*----------------------------------------------------------------------------*/
static void checksumRequest(struct SdioSpi *interface)
{
  ifRead(interface->interface, interface->buffer, 2);
}
/*----------------------------------------------------------------------------*/
static void readDataBlock(struct SdioSpi *interface)
{
  ifRead(interface->interface, interface->rxBuffer, interface->blockLength);
}
/*----------------------------------------------------------------------------*/
static void responseProcess(struct SdioSpi *interface, enum sdioResponse type)
{
  /* Responses come in big-endian format */
  if (type == SDIO_RESPONSE_SHORT)
  {
    interface->response[0] = fromBigEndian32(*(uint32_t *)interface->buffer);
  }
  else
  {
    const uint32_t *buffer = (const uint32_t *)interface->buffer;

    for (uint8_t index = 0; index < 4; ++index)
      interface->response[3 - index] = fromBigEndian32(buffer[index]);
  }
}
/*----------------------------------------------------------------------------*/
static void responseRequest(struct SdioSpi *interface, enum sdioResponse type)
{
  if (type == SDIO_RESPONSE_SHORT)
  {
    /* Read 32-bit response */
    ifRead(interface->interface, interface->buffer, 4);
  }
  else
  {
    /* Read 128-bit response */
    ifRead(interface->interface, interface->buffer, 16);
  }
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
static void sendCommand(struct SdioSpi *interface)
{
  const uint32_t argument = toBigEndian32(interface->argument);
  const uint8_t code = COMMAND_CODE_VALUE(interface->command);

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

  pinReset(interface->cs);
  ifWrite(interface->interface, interface->buffer, 8);
}
/*----------------------------------------------------------------------------*/
static void sendInitSequence(struct SdioSpi *interface)
{
  memset(interface->buffer, 0xFF, 10);
  ifWrite(interface->interface, interface->buffer, 10);
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
static void tokenRequest(struct SdioSpi *interface)
{
  ifRead(interface->interface, interface->buffer, 1);
}
/*----------------------------------------------------------------------------*/
static void interruptHandler(void *object)
{
  struct SdioSpi * const interface = object;
  enum result res;

  pinSet(interface->debug1);
  switch (interface->state)
  {
    case SDIO_SPI_STATE_INIT:
      sendCommand(interface);
      interface->state = SDIO_SPI_STATE_SEND_CMD;
      break;

    case SDIO_SPI_STATE_SEND_CMD:
      tokenRequest(interface);
      interface->iteration = 0;
      interface->state = SDIO_SPI_STATE_WAIT_RESPONSE;
      break;

    case SDIO_SPI_STATE_WAIT_RESPONSE:
      res = responseTokenProcess(interface);
      if (res == E_BUSY)
      {
        /* Device is busy, try again */
        tokenRequest(interface);
        ++interface->iteration;
      }
      else if (res == E_OK || res == E_IDLE)
      {
        const uint16_t flags = COMMAND_FLAG_VALUE(interface->command);

        switch (COMMAND_RESP_VALUE(interface->command))
        {
          case SDIO_RESPONSE_SHORT:
            responseRequest(interface, SDIO_RESPONSE_SHORT);
            interface->tokenStatus = res;
            interface->state = SDIO_SPI_STATE_READ_SHORT;
            break;

          case SDIO_RESPONSE_LONG:
            tokenRequest(interface);
            interface->iteration = 0;
            interface->tokenStatus = res;
            interface->state = SDIO_SPI_STATE_WAIT_LONG;
            break;

          default:
          {
            if (!(flags & SDIO_DATA_MODE))
            {
              /* No response expected */
              goto event;
            }

            if (flags & SDIO_READ_WRITE)
            {
              /* Write data mode */
            }
            else
            {
              /* Read data mode */
              tokenRequest(interface);
              interface->iteration = 0;
              interface->state = SDIO_SPI_STATE_WAIT_READ_TOKEN;
            }
          }
        }
      }
      else
      {
        goto event;
      }
      break;

    case SDIO_SPI_STATE_READ_SHORT:
      responseProcess(interface, SDIO_RESPONSE_SHORT);
      res = interface->tokenStatus;
      goto event;

    case SDIO_SPI_STATE_WAIT_LONG:
      res = tokenProcess(interface, TOKEN_START);
      if (res == E_BUSY)
      {
        /* Device is busy, try again */
        tokenRequest(interface);
        ++interface->iteration;
      }
      else if (res == E_OK)
      {
        responseRequest(interface, SDIO_RESPONSE_LONG);
        interface->state = SDIO_SPI_STATE_READ_LONG;
      }
      else
      {
        goto event;
      }
      break;

    case SDIO_SPI_STATE_READ_LONG:
      responseProcess(interface, SDIO_RESPONSE_LONG);
      checksumRequest(interface);
      interface->state = SDIO_SPI_STATE_READ_LONG_CRC;
      break;

    case SDIO_SPI_STATE_READ_LONG_CRC:
      //TODO Merge with previous state
      interface->status = checksumProcess(interface, 0x0000); //TODO
      goto event;

    case SDIO_SPI_STATE_WAIT_READ_TOKEN:
      res = tokenProcess(interface, TOKEN_START);
      if (res == E_BUSY)
      {
        /* Device is busy, try again */
        tokenRequest(interface);
        ++interface->iteration;
      }
      else if (res == E_OK)
      {
        readDataBlock(interface);
        interface->left -= interface->blockLength;
        interface->state = SDIO_SPI_STATE_READ_DATA;
      }
      else
      {
        goto event;
      }
      break;

    case SDIO_SPI_STATE_READ_DATA:
      checksumRequest(interface);
      interface->state = SDIO_SPI_STATE_READ_CRC;
      break;

    case SDIO_SPI_STATE_READ_CRC:
      if (COMMAND_FLAG_VALUE(interface->command) & SDIO_CHECK_CRC)
      {
        //TODO Rewrite
        if ((res = checksumProcess(interface, 0x0000)) != E_OK)
          goto event;
      }
      if (interface->left)
      {
        /* Continue to read data */
        tokenRequest(interface);
        interface->iteration = 0;
        interface->rxBuffer += interface->blockLength;
        interface->state = SDIO_SPI_STATE_WAIT_READ_TOKEN;
      }
      else
      {
        res = E_OK;
        goto event;
      }

    default:
      break;
  }

  pinReset(interface->debug1); //TODO Remove
  return;

event:
  pinReset(interface->debug1); //TODO Remove
  /* End of transfer */
  interface->status = res;
  interface->state = SDIO_SPI_STATE_IDLE;

  pinReset(interface->debug2); //TODO Remove
  pinSet(interface->cs);
  ifSet(interface->interface, IF_RELEASE, 0);

  if (interface->callback)
    interface->callback(interface->callbackArgument);
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
  interface->debug1 = pinInit(PIN(1, 0));
  pinOutput(interface->debug1, 0);
  interface->debug2 = pinInit(PIN(2, 10));
  pinOutput(interface->debug2, 0);

  interface->blockLength = 512; /* 512 bytes by default */
  interface->left = 0;
  interface->rxBuffer = 0;
  interface->txBuffer = 0;

  interface->argument = 0;
  interface->blocking = true;
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

  /* Check buffer alignment */
  assert(!(length % interface->blockLength));

  interface->status = E_BUSY;
  ifSet(interface->interface, IF_ACQUIRE, 0);
  pinSet(interface->debug2); //TODO Remove

  interface->left = length;
  interface->rxBuffer = buffer;
  interface->state = SDIO_SPI_STATE_SEND_CMD;
  sendCommand(interface);

  return length;
}
/*----------------------------------------------------------------------------*/
static uint32_t sdioWrite(void *object, const uint8_t *buffer, uint32_t length)
{
  struct SdioSpi * const interface = object;

  /* Check buffer alignment */
  assert(!(length % interface->blockLength));

  return length;
}
