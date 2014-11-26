/*
 * sdio_spi.c
 * Copyright (C) 2014 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <assert.h>
#include <string.h>
#include <memory.h>
#include <modules/sdio.h>
#include <modules/sdio_defs.h>
#include <platform/nxp/sdio_spi.h>
/*----------------------------------------------------------------------------*/
#define BUSY_RETRIES  10000
#define TOKEN_RETRIES 8
/*----------------------------------------------------------------------------*/
enum state
{
  STATE_IDLE,
  STATE_INIT,
  STATE_SEND_CMD,
  STATE_WAIT_RESP,
  STATE_READ_SHORT,
  STATE_WAIT_LONG,
  STATE_READ_LONG,
  STATE_WAIT_READ,
  STATE_READ_DATA,
  STATE_READ_CRC,
  STATE_WAIT_BUSY
};
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
struct StateEntry
{
  void (*enter)(struct SdioSpi *);
  enum state (*advance)(struct SdioSpi *);
  uint8_t next;
};
/*----------------------------------------------------------------------------*/
static void stateInitEnter(struct SdioSpi *);
static void stateSendCommandEnter(struct SdioSpi *);
static void stateWaitRespEnter(struct SdioSpi *);
static enum state stateWaitRespAdvance(struct SdioSpi *);
static void stateReadShortEnter(struct SdioSpi *);
static enum state stateReadShortAdvance(struct SdioSpi *);
static void stateWaitLongEnter(struct SdioSpi *);
static enum state stateWaitLongAdvance(struct SdioSpi *);
static void stateReadLongEnter(struct SdioSpi *);
static enum state stateReadLongAdvance(struct SdioSpi *);
static void stateWaitReadEnter(struct SdioSpi *);
static enum state stateWaitReadAdvance(struct SdioSpi *);
static void stateReadDataEnter(struct SdioSpi *);
static void stateReadCrcEnter(struct SdioSpi *);
static enum state stateReadCrcAdvance(struct SdioSpi *);
/*----------------------------------------------------------------------------*/
static void execute(struct SdioSpi *);
static void interruptHandler(void *);
static enum result parseDataToken(struct SdioSpi *, uint8_t, enum sdioToken);
static enum result parseResponseToken(struct SdioSpi *, uint8_t);
static void sendCommand(struct SdioSpi *, uint32_t, uint32_t);
/*----------------------------------------------------------------------------*/
static enum result sdioInit(void *, const void *);
static void sdioDeinit(void *);
static enum result sdioCallback(void *, void (*)(void *), void *);
static enum result sdioGet(void *, enum ifOption, void *);
static enum result sdioSet(void *, enum ifOption, const void *);
static uint32_t sdioRead(void *, uint8_t *, uint32_t);
static uint32_t sdioWrite(void *, const uint8_t *, uint32_t);
/*----------------------------------------------------------------------------*/
static const struct StateEntry stateTable[] = {
    [STATE_IDLE]        = {0, 0, STATE_IDLE},
    [STATE_INIT]        = {stateInitEnter, 0, STATE_SEND_CMD},
    [STATE_SEND_CMD]    = {stateSendCommandEnter, 0, STATE_WAIT_RESP},
    [STATE_WAIT_RESP]   = {stateWaitRespEnter, stateWaitRespAdvance, 0},
    [STATE_READ_SHORT]  = {stateReadShortEnter, stateReadShortAdvance, 0},
    [STATE_WAIT_LONG]   = {stateWaitLongEnter, stateWaitLongAdvance, 0},
    [STATE_READ_LONG]   = {stateReadLongEnter, stateReadLongAdvance, 0},
    [STATE_WAIT_READ]   = {stateWaitReadEnter, stateWaitReadAdvance, 0},
    [STATE_READ_DATA]   = {stateReadDataEnter, 0, STATE_READ_CRC},
    [STATE_READ_CRC]    = {stateReadCrcEnter, stateReadCrcAdvance, 0},
    [STATE_WAIT_BUSY]   = {0, 0, STATE_IDLE}
};
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
static void stateInitEnter(struct SdioSpi *interface)
{
  memset(interface->buffer, 0xFF, 10);
  ifWrite(interface->interface, interface->buffer, 10);
}
/*----------------------------------------------------------------------------*/
static void stateSendCommandEnter(struct SdioSpi *interface)
{
  pinReset(interface->cs);
  sendCommand(interface, interface->command, interface->argument);
  interface->retries = TOKEN_RETRIES;
}
/*----------------------------------------------------------------------------*/
static void stateWaitRespEnter(struct SdioSpi *interface)
{
  ifRead(interface->interface, interface->buffer, 1);
  --interface->retries;
}
/*----------------------------------------------------------------------------*/
static enum state stateWaitRespAdvance(struct SdioSpi *interface)
{
  const enum result res = parseResponseToken(interface, interface->buffer[0]);

  if (res == E_OK || res == E_IDLE)
  {
    const uint16_t flags = COMMAND_FLAG_VALUE(interface->command);

    if (flags & SDIO_DATA_MODE)
    {
      if (flags & SDIO_READ_WRITE)
      {
        /* Write data mode */
        //TODO
      }
      else
      {
        /* Read data mode */
        interface->retries = TOKEN_RETRIES;
        return STATE_WAIT_READ;
      }
    }

    switch (COMMAND_RESP_VALUE(interface->command))
    {
      case SDIO_RESPONSE_SHORT:
        interface->tokenStatus = res;
        return STATE_READ_SHORT;

      case SDIO_RESPONSE_LONG:
        interface->retries = TOKEN_RETRIES;
        return STATE_WAIT_LONG;

      default:
        interface->status = res;
        return STATE_IDLE;
    }
  }
  else if (res == E_BUSY)
  {
    /* Interface is busy, try again */
    return STATE_WAIT_RESP;
  }
  else
  {
    /* Error occurred, stop the transfer */
    interface->status = res;
    return STATE_IDLE;
  }
}
/*----------------------------------------------------------------------------*/
static void stateReadShortEnter(struct SdioSpi *interface)
{
  /* Read 32-bit response */
  ifRead(interface->interface, interface->buffer, 4);
}
/*----------------------------------------------------------------------------*/
static enum state stateReadShortAdvance(struct SdioSpi *interface)
{
  interface->response[0] = fromBigEndian32(*(uint32_t *)interface->buffer);
  interface->status = interface->tokenStatus;
  return STATE_IDLE;
}
/*----------------------------------------------------------------------------*/
static void stateWaitLongEnter(struct SdioSpi *interface)
{
  ifRead(interface->interface, interface->buffer, 1);
  --interface->retries;
}
/*----------------------------------------------------------------------------*/
static enum state stateWaitLongAdvance(struct SdioSpi *interface)
{
  const enum result res = parseDataToken(interface, interface->buffer[0],
      TOKEN_START);

  if (res == E_OK)
  {
    return STATE_READ_LONG;
  }
  else if (res == E_BUSY)
  {
    /* Interface is busy, try again */
    return STATE_WAIT_LONG;
  }
  else
  {
    /* Error occurred, stop the transfer */
    interface->status = res;
    return STATE_IDLE;
  }
}
/*----------------------------------------------------------------------------*/
static void stateReadLongEnter(struct SdioSpi *interface)
{
  /* Read 128-bit response and 16-bit checksum */
  ifRead(interface->interface, interface->buffer, 18);
}
/*----------------------------------------------------------------------------*/
static enum state stateReadLongAdvance(struct SdioSpi *interface)
{
  const uint32_t * const buffer = (const uint32_t *)interface->buffer;

  for (uint8_t index = 0; index < 4; ++index)
    interface->response[3 - index] = fromBigEndian32(buffer[index]);

  //TODO Add CRC checking
  interface->status = E_OK;
  return STATE_IDLE;
}
/*----------------------------------------------------------------------------*/
static void stateWaitReadEnter(struct SdioSpi *interface)
{
  ifRead(interface->interface, interface->buffer, 1);
  --interface->retries;
}
/*----------------------------------------------------------------------------*/
static enum state stateWaitReadAdvance(struct SdioSpi *interface)
{
  const enum result res = parseDataToken(interface, interface->buffer[0],
      TOKEN_START);

  if (res == E_OK)
  {
    return STATE_READ_DATA;
  }
  else if (res == E_BUSY)
  {
    /* Interface is busy, try again */
    return STATE_WAIT_READ;
  }
  else
  {
    /* Error occurred, stop the transfer */
    interface->status = res;
    return STATE_IDLE;
  }
}
/*----------------------------------------------------------------------------*/
static void stateReadDataEnter(struct SdioSpi *interface)
{
  ifRead(interface->interface, interface->rxBuffer, interface->blockLength);
  interface->left -= interface->blockLength;
}
/*----------------------------------------------------------------------------*/
static void stateReadCrcEnter(struct SdioSpi *interface)
{
  ifRead(interface->interface, interface->buffer, 2);
}
/*----------------------------------------------------------------------------*/
static enum state stateReadCrcAdvance(struct SdioSpi *interface)
{
  const uint16_t flags = COMMAND_FLAG_VALUE(interface->command);

  if (flags & SDIO_CHECK_CRC)
  {
    //TODO Add CRC checking
  }

  /* Increase pointer to an input buffer */
  interface->rxBuffer += interface->blockLength;

  if (interface->left)
  {
    /* Continue to read data */
    interface->retries = TOKEN_RETRIES;
    return STATE_WAIT_READ;
  }
  else if (flags & SDIO_AUTO_STOP)
  {
    /*
     * Inject Stop Transmission command when Auto Stop flag is set.
     * Command injection changes current command code. This behavior does not
     * interfere with interface response type because response parameters
     * are similar to ones in the original command.
     */
    interface->command = SDIO_COMMAND(CMD_STOP_TRANSMISSION,
        SDIO_RESPONSE_NONE, 0);
    interface->argument = 0;

    sendCommand(interface, interface->command, interface->argument);
    interface->retries = TOKEN_RETRIES;
    return STATE_SEND_CMD;
  }
  else
  {
    /* No more data required, stop the transfer */
    interface->status = E_OK;
    return STATE_IDLE;
  }
}
/*----------------------------------------------------------------------------*/
static void execute(struct SdioSpi *interface)
{
  const uint16_t flags = COMMAND_FLAG_VALUE(interface->command);

  interface->status = E_BUSY;
  ifSet(interface->interface, IF_ACQUIRE, 0);

  if (flags & SDIO_INITIALIZE)
  {
    interface->state = STATE_INIT;
    stateTable[STATE_INIT].enter(interface);
  }
  else
  {
    interface->state = STATE_SEND_CMD;
    stateTable[STATE_SEND_CMD].enter(interface);
  }
}
/*----------------------------------------------------------------------------*/
static void interruptHandler(void *object)
{
  struct SdioSpi * const interface = object;
  enum state next;

  if (stateTable[interface->state].advance)
    next = stateTable[interface->state].advance(interface);
  else
    next = stateTable[interface->state].next;

  interface->state = next;

  if (interface->state != STATE_IDLE)
  {
    if (stateTable[interface->state].enter)
      stateTable[interface->state].enter(interface);
  }
  else
  {
    /* Finalize the transfer */
    pinSet(interface->cs);
    ifSet(interface->interface, IF_RELEASE, 0);

    if (interface->callback)
      interface->callback(interface->callbackArgument);
  }
}
/*----------------------------------------------------------------------------*/
static enum result parseDataToken(struct SdioSpi *interface, uint8_t token,
    enum sdioToken expected)
{
  enum result res;

  if (token == 0xFF)
    res = !interface->retries ? E_TIMEOUT : E_BUSY;
  else if (token != expected)
    res = E_DEVICE;
  else
    res = E_OK;

  return res;
}
/*----------------------------------------------------------------------------*/
static enum result parseResponseToken(struct SdioSpi *interface, uint8_t token)
{
  enum result res;

  if (token == 0xFF)
    res = !interface->retries ? E_TIMEOUT : E_BUSY;
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
static void sendCommand(struct SdioSpi *interface, uint32_t command,
    uint32_t argument)
{
  argument = toBigEndian32(argument);

  /* Fill the buffer */
  interface->buffer[0] = 0xFF;
  interface->buffer[1] = 0x40 | COMMAND_CODE_VALUE(command);
  memcpy(interface->buffer + 2, &argument, sizeof(argument));

  // TODO Remove hardcoded values
  /* Checksum should be valid only for first CMD0 and CMD8 commands */
  switch (COMMAND_CODE_VALUE(command))
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

  ifWrite(interface->interface, interface->buffer, 8);
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

  interface->blockLength = 512; /* 512 bytes by default */
  interface->left = 0;
  interface->rxBuffer = 0;
  interface->txBuffer = 0;

  interface->argument = 0;
  interface->blocking = true;
  interface->callback = 0;
  interface->command = 0;
  interface->state = STATE_IDLE;
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

  interface->left = length;
  interface->rxBuffer = buffer;

  /* Begin execution */
  interface->state = STATE_SEND_CMD;
  stateTable[STATE_SEND_CMD].enter(interface);

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
