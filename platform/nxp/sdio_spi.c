/*
 * sdio_spi.c
 * Copyright (C) 2014 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <memory.h>
#include <common/sdio.h>
#include <common/sdio_defs.h>
#include <crc/crc7.h>
#include <crc/crc16.h>
#include <platform/nxp/sdio_spi.h>
/*----------------------------------------------------------------------------*/
#define DEFAULT_BLOCK_SIZE    512
#define BUSY_TIMER_FREQUENCY  100000
#define BUSY_READ_DELAY       10 /* Delay in timer ticks */
#define BUSY_READ_RETRIES     1000
#define BUSY_WRITE_DELAY      50 /* Delay in timer ticks */
#define BUSY_WRITE_RETRIES    1000
#define INITIAL_CRC7          0x00
#define INITIAL_CRC16         0x0000
#define TOKEN_RETRIES         8
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
  STATE_READ_DELAY,
  STATE_WRITE_TOKEN,
  STATE_WRITE_DATA,
  STATE_WRITE_CRC,
  STATE_WAIT_WRITE,
  STATE_WRITE_DELAY,
  STATE_WRITE_BUSY,
  STATE_WRITE_STOP
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
static void stateRequestToken(struct SdioSpi *);
static enum state stateWaitLongAdvance(struct SdioSpi *);
static void stateReadLongEnter(struct SdioSpi *);
static enum state stateReadLongAdvance(struct SdioSpi *);
static enum state stateWaitReadAdvance(struct SdioSpi *);
static void stateReadDataEnter(struct SdioSpi *);
static void stateReadCrcEnter(struct SdioSpi *);
static enum state stateReadCrcAdvance(struct SdioSpi *);
static void stateDelayEnter(struct SdioSpi *);
static enum state stateReadDelayAdvance(struct SdioSpi *);
static void stateWriteTokenEnter(struct SdioSpi *);
static void stateWriteDataEnter(struct SdioSpi *);
static void stateWriteCrcEnter(struct SdioSpi *);
static enum state stateWaitWriteAdvance(struct SdioSpi *);
static enum state stateWriteDelayAdvance(struct SdioSpi *);
static enum state stateWriteBusyAdvance(struct SdioSpi *);
static void stateWriteStopEnter(struct SdioSpi *);
/*----------------------------------------------------------------------------*/
static void execute(struct SdioSpi *);
static void interruptHandler(void *);
static enum result parseDataToken(struct SdioSpi *, uint8_t, enum sdioToken);
static enum result parseResponseToken(struct SdioSpi *, uint8_t);
static void sendCommand(struct SdioSpi *, uint32_t, uint32_t);
static enum result verifyChecksum(struct SdioSpi *);
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
    [STATE_WAIT_LONG]   = {stateRequestToken, stateWaitLongAdvance, 0},
    [STATE_READ_LONG]   = {stateReadLongEnter, stateReadLongAdvance, 0},
    [STATE_WAIT_READ]   = {stateRequestToken, stateWaitReadAdvance, 0},
    [STATE_READ_DATA]   = {stateReadDataEnter, 0, STATE_READ_CRC},
    [STATE_READ_CRC]    = {stateReadCrcEnter, stateReadCrcAdvance, 0},
    [STATE_READ_DELAY]  = {stateDelayEnter, stateReadDelayAdvance, 0},
    [STATE_WRITE_TOKEN] = {stateWriteTokenEnter, 0, STATE_WRITE_DATA},
    [STATE_WRITE_DATA]  = {stateWriteDataEnter, 0, STATE_WRITE_CRC},
    [STATE_WRITE_CRC]   = {stateWriteCrcEnter, 0, STATE_WAIT_WRITE},
    [STATE_WAIT_WRITE]  = {stateRequestToken, stateWaitWriteAdvance, 0},
    [STATE_WRITE_DELAY] = {stateDelayEnter, stateWriteDelayAdvance, 0},
    [STATE_WRITE_BUSY]  = {stateRequestToken, stateWriteBusyAdvance, 0},
    [STATE_WRITE_STOP]  = {stateWriteStopEnter, 0, STATE_WRITE_BUSY}
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
  ifWrite(interface->bus, interface->buffer, 10);
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
  ifRead(interface->bus, interface->buffer, 1);
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
      if (flags & SDIO_WRITE_MODE)
      {
        /* Write data mode */
        if (interface->timer)
          timerSetOverflow(interface->timer, BUSY_WRITE_DELAY);

        return STATE_WRITE_TOKEN;
      }
      else
      {
        /* Read data mode */
        if (interface->timer)
          timerSetOverflow(interface->timer, BUSY_READ_DELAY);

        interface->retries = BUSY_READ_RETRIES;
        return STATE_WAIT_READ;
      }
    }

    switch (COMMAND_RESP_VALUE(interface->command))
    {
      case SDIO_RESPONSE_SHORT:
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
  /* Read 32-bit response into temporary buffer, command token is preserved */
  ifRead(interface->bus, interface->buffer + 1, 4);
}
/*----------------------------------------------------------------------------*/
static enum state stateReadShortAdvance(struct SdioSpi *interface)
{
  uint32_t value;

  /* Command token is preserved in the first byte of data buffer */
  memcpy(&value, interface->buffer + 1, 4);
  interface->response[0] = fromBigEndian32(value);
  interface->status = parseResponseToken(interface, interface->buffer[0]);

  return STATE_IDLE;
}
/*----------------------------------------------------------------------------*/
static void stateRequestToken(struct SdioSpi *interface)
{
  ifRead(interface->bus, interface->buffer, 1);
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
  ifRead(interface->bus, interface->buffer, 18);
}
/*----------------------------------------------------------------------------*/
static enum state stateReadLongAdvance(struct SdioSpi *interface)
{
  const uint32_t * const buffer = (const uint32_t *)interface->buffer;

  for (uint8_t index = 0; index < 4; ++index)
    interface->response[3 - index] = fromBigEndian32(buffer[index]);

  interface->status = E_OK;
  return STATE_IDLE;
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
    return interface->timer ? STATE_READ_DELAY : STATE_WAIT_READ;
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
  ifRead(interface->bus, interface->rxBuffer
      + (interface->length - interface->left), interface->blockSize);
  interface->left -= interface->blockSize;
}
/*----------------------------------------------------------------------------*/
static void stateReadCrcEnter(struct SdioSpi *interface)
{
  ifRead(interface->bus, interface->buffer, 2);
}
/*----------------------------------------------------------------------------*/
static enum state stateReadCrcAdvance(struct SdioSpi *interface)
{
  const uint16_t flags = COMMAND_FLAG_VALUE(interface->command);

  if (flags & SDIO_CHECK_CRC)
  {
    const uint16_t blockIndex = (interface->length - interface->left)
        / interface->blockSize - 1;
    uint16_t receivedCrc;

    memcpy(&receivedCrc, interface->buffer, 2);
    receivedCrc = fromBigEndian16(receivedCrc);
    interface->crcPool[blockIndex] = receivedCrc;
  }

  if (interface->left)
  {
    /* Continue to read data */
    interface->retries = BUSY_READ_RETRIES;
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
    interface->status = E_OK;

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
static void stateDelayEnter(struct SdioSpi *interface)
{
  timerSetValue(interface->timer, 0);
  timerSetEnabled(interface->timer, true);
  --interface->retries;
}
/*----------------------------------------------------------------------------*/
static enum state stateReadDelayAdvance(struct SdioSpi *interface)
{
  timerSetEnabled(interface->timer, false);
  return STATE_WAIT_READ;
}
/*----------------------------------------------------------------------------*/
static void stateWriteTokenEnter(struct SdioSpi *interface)
{
  /* Auto stop is enabled only during multiple block write */
  const uint16_t flags = COMMAND_FLAG_VALUE(interface->command);
  interface->buffer[0] = flags & SDIO_AUTO_STOP ? TOKEN_START_MULTIPLE
      : TOKEN_START;

  interface->retries = TOKEN_RETRIES;
  ifWrite(interface->bus, interface->buffer, 1);
}
/*----------------------------------------------------------------------------*/
static void stateWriteDataEnter(struct SdioSpi *interface)
{
  ifWrite(interface->bus, interface->txBuffer
      + (interface->length - interface->left), interface->blockSize);
  interface->left -= interface->blockSize;
}
/*----------------------------------------------------------------------------*/
static void stateWriteCrcEnter(struct SdioSpi *interface)
{
  const uint16_t flags = COMMAND_FLAG_VALUE(interface->command);
  uint16_t computedCrc;

  if (flags & SDIO_CHECK_CRC)
  {
    const uint16_t blockIndex = (interface->length - interface->left)
          / interface->blockSize - 1;

    computedCrc = interface->crcPool[blockIndex];
  }
  else
  {
    computedCrc = 0xFFFF;
  }

  computedCrc = toBigEndian16(computedCrc);
  memcpy(interface->buffer, &computedCrc, 2);
  ifWrite(interface->bus, interface->buffer, 2);
}
/*----------------------------------------------------------------------------*/
static enum state stateWaitWriteAdvance(struct SdioSpi *interface)
{
  const uint8_t token = interface->buffer[0] & 0x1F;
  const enum result res = parseDataToken(interface, token, TOKEN_DATA_ACCEPTED);

  if (res == E_OK)
  {
    interface->retries = BUSY_WRITE_RETRIES;
    return STATE_WRITE_BUSY;
  }
  else if (res == E_BUSY)
  {
    /* Interface is busy, try again */
    return STATE_WAIT_WRITE;
  }
  else
  {
    /* Error occurred, stop the transfer */
    interface->status = res;
    return STATE_IDLE;
  }
}
/*----------------------------------------------------------------------------*/
static enum state stateWriteDelayAdvance(struct SdioSpi *interface)
{
  timerSetEnabled(interface->timer, false);
  return STATE_WRITE_BUSY;
}
/*----------------------------------------------------------------------------*/
static enum state stateWriteBusyAdvance(struct SdioSpi *interface)
{
  if (interface->buffer[0] != 0xFF)
  {
    if (!interface->retries)
    {
      /* Error occurred, stop the transfer */
      interface->status = E_TIMEOUT;
      return STATE_IDLE;
    }
    else
      return interface->timer ? STATE_WRITE_DELAY : STATE_WRITE_BUSY;
  }
  else
  {
    if (interface->left)
      return STATE_WRITE_TOKEN;

    const uint16_t flags = COMMAND_FLAG_VALUE(interface->command);

    if ((flags & SDIO_AUTO_STOP) && interface->txBuffer)
    {
      interface->retries = BUSY_WRITE_RETRIES;
      interface->txBuffer = 0;
      return STATE_WRITE_STOP;
    }
    else
    {
      interface->status = E_OK;
      return STATE_IDLE;
    }
  }
}
/*----------------------------------------------------------------------------*/
static void stateWriteStopEnter(struct SdioSpi *interface)
{
  interface->buffer[0] = TOKEN_STOP;
  ifWrite(interface->bus, interface->buffer, 1);
}
/*----------------------------------------------------------------------------*/
static void execute(struct SdioSpi *interface)
{
  ifSet(interface->bus, IF_ACQUIRE, 0);

  const uint16_t flags = COMMAND_FLAG_VALUE(interface->command);

  interface->status = interface->transferStatus = E_BUSY;

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
    if (interface->transferStatus == E_BUSY)
    {
      /*
       * Copy status of the last command when the global command status
       * is not changed. Global command status may be changed when
       * one of low-level commands has failed.
       */
      interface->transferStatus = interface->status;
    }

    if (interface->checkReceivedCrc && interface->transferStatus != E_OK)
      interface->checkReceivedCrc = false;

    /* Finalize the transfer */
    pinSet(interface->cs);
    ifSet(interface->bus, IF_RELEASE, 0);

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

  interface->buffer[0] = 0xFF;
  interface->buffer[1] = COMMAND_CODE_VALUE(command) | 0x40;
  memcpy(interface->buffer + 2, &argument, sizeof(argument));
  interface->buffer[6] = crcUpdate(interface->crc7, INITIAL_CRC7,
      interface->buffer + 1, 5);
  /* Add end bit */
  interface->buffer[6] = (interface->buffer[6] << 1) | 0x01;
  interface->buffer[7] = 0xFF;

  ifWrite(interface->bus, interface->buffer, 8);
}
/*----------------------------------------------------------------------------*/
static enum result verifyChecksum(struct SdioSpi *interface)
{
  const uint16_t blockCount = interface->length / interface->blockSize;
  enum result res = E_OK;

  for (uint16_t index = 0; index < blockCount; ++index)
  {
    const uint16_t computed = crcUpdate(interface->crc16, INITIAL_CRC16,
        interface->rxBuffer + index * interface->blockSize,
        interface->blockSize);

    if (computed != interface->crcPool[index])
    {
      res = E_INTERFACE;
      break;
    }
  }

  return res;
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

  interface->bus = config->interface;

  if ((res = ifSet(interface->bus, IF_ZEROCOPY, 0)) != E_OK)
    return res;
  if ((res = ifCallback(interface->bus, interruptHandler, interface)) != E_OK)
    return res;

  interface->timer = config->timer;

  if (interface->timer)
  {
    /* TODO Add priority configuration */
    timerSetEnabled(interface->timer, false);
    timerCallback(interface->timer, interruptHandler, interface);

    res = timerSetFrequency(interface->timer, BUSY_TIMER_FREQUENCY);
    if (res != E_OK)
      return res;
  }

  /* Command verification part */
  interface->crc7 = init(Crc7, 0);
  if (!interface->crc7)
    return E_MEMORY;

  /* Data verification part */
  interface->checkReceivedCrc = false;
  interface->crcPoolSize = config->blocks;
  if (interface->crcPoolSize)
  {
    interface->crc16 = init(Crc16, 0);
    if (!interface->crc16)
      return E_MEMORY;

    interface->crcPool = malloc(2 * interface->crcPoolSize);
    if (!interface->crcPool)
      return E_MEMORY;
  }
  else
  {
    interface->crc16 = 0;
    interface->crcPool = 0;
  }

  /* Data transfer part */
  interface->blockSize = DEFAULT_BLOCK_SIZE;
  interface->left = 0;
  interface->length = 0;
  interface->rxBuffer = 0;
  interface->txBuffer = 0;

  /* Command execution part */
  interface->argument = 0;
  interface->callback = 0;
  interface->command = 0;
  interface->state = STATE_IDLE;
  interface->status = interface->transferStatus = E_OK;
  memset(interface->response, 0, sizeof(interface->response));

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static void sdioDeinit(void *object __attribute__((unused)))
{
  struct SdioSpi * const interface = object;

  if (interface->timer)
    timerCallback(interface->timer, 0, 0);

  ifCallback(interface->bus, 0, 0);
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
      const uint8_t length = response == SDIO_RESPONSE_LONG ? 16 : 4;

      if (response == SDIO_RESPONSE_NONE)
        return E_ERROR;

      memcpy(data, interface->response, length);
      return E_OK;
    }

    default:
      break;
  }

  switch (option)
  {
    case IF_RATE:
      return ifGet(interface->bus, IF_RATE, data);

    case IF_STATUS:
    {
      if (interface->state == STATE_IDLE)
      {
        if (interface->transferStatus == E_OK && interface->checkReceivedCrc)
        {
          interface->checkReceivedCrc = false;
          interface->transferStatus = verifyChecksum(interface);
        }

        return interface->transferStatus;
      }
      else
        return E_BUSY;
    }

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

    case IF_SDIO_BLOCK_SIZE:
      interface->blockSize = (uint16_t)(*(const uint32_t *)data);
      return E_OK;

    case IF_SDIO_COMMAND:
      interface->command = *(const uint32_t *)data;
      return E_OK;

    default:
      break;
  }

  switch (option)
  {
    case IF_RATE:
      return ifSet(interface->bus, IF_RATE, data);

    default:
      return E_ERROR;
  }
}
/*----------------------------------------------------------------------------*/
static uint32_t sdioRead(void *object, uint8_t *buffer, uint32_t length)
{
  struct SdioSpi * const interface = object;

  ifSet(interface->bus, IF_ACQUIRE, 0);

  const uint16_t flags = COMMAND_FLAG_VALUE(interface->command);

  /* Check buffer alignment */
  assert(!(length & (interface->blockSize - 1)));

  if (flags & SDIO_CHECK_CRC)
  {
    if (interface->crcPoolSize >= length / interface->blockSize)
    {
      interface->checkReceivedCrc = true;
    }
    else
    {
      /* Checksum pool is not big enough */
      interface->status = E_MEMORY;
      return 0;
    }
  }

  interface->status = interface->transferStatus = E_BUSY;

  interface->left = length;
  interface->length = length;
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

  ifSet(interface->bus, IF_ACQUIRE, 0);

  const uint16_t flags = COMMAND_FLAG_VALUE(interface->command);

  /* Check buffer alignment */
  assert(!(length & (interface->blockSize - 1)));

  if (flags & SDIO_CHECK_CRC)
  {
    const uint16_t blockCount = length / interface->blockSize;

    if (interface->crcPoolSize >= blockCount)
    {
      for (uint16_t index = 0; index < blockCount; ++index)
      {
        interface->crcPool[index] = crcUpdate(interface->crc16, INITIAL_CRC16,
            buffer + index * interface->blockSize, interface->blockSize);
      }
    }
    else
    {
      /* Checksum pool is not big enough */
      interface->status = E_MEMORY;
      return 0;
    }
  }

  interface->status = interface->transferStatus = E_BUSY;

  interface->left = length;
  interface->length = length;
  interface->txBuffer = buffer;

  /* Begin execution */
  interface->state = STATE_SEND_CMD;
  stateTable[STATE_SEND_CMD].enter(interface);

  return length;
}
