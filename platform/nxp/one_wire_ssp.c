/*
 * one_wire_ssp.c
 * Copyright (C) 2014 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <xcore/memory.h>
#include <halm/platform/nxp/one_wire_ssp.h>
#include <halm/platform/nxp/ssp_defs.h>
/*----------------------------------------------------------------------------*/
#define DATA_MASK         0x3FC0

#define PATTERN_HIGH      0x7FFF
#define PATTERN_LOW       0x000F
#define PATTERN_PRESENCE  0xFFFF
#define PATTERN_RESET     0x0000

#define RATE_RESET        31250
#define RATE_DATA         250000

#define TX_QUEUE_LENGTH   24
/*----------------------------------------------------------------------------*/
enum Command
{
  SEARCH_ROM  = 0xF0,
  READ_ROM    = 0x33,
  MATCH_ROM   = 0x55,
  SKIP_ROM    = 0xCC
};
/*----------------------------------------------------------------------------*/
enum State
{
  STATE_IDLE,
  STATE_RESET,
  STATE_PRESENCE,
  STATE_RECEIVE,
  STATE_TRANSMIT,
  STATE_SEARCH_START,
  STATE_SEARCH_REQUEST,
  STATE_SEARCH_RESPONSE,
  STATE_ERROR
};
/*----------------------------------------------------------------------------*/
static void adjustPins(struct OneWireSsp *, const struct OneWireSspConfig *);
static void beginTransmission(struct OneWireSsp *);
static void sendWord(struct OneWireSsp *, uint8_t);
static void standardInterruptHandler(void *);
/*----------------------------------------------------------------------------*/
#ifdef CONFIG_PLATFORM_NXP_ONE_WIRE_SSP_SEARCH
static void searchInterruptHandler(void *);
static void sendSearchRequest(struct OneWireSsp *);
static void sendSearchResponse(struct OneWireSsp *, bool);
static void startSearch(struct OneWireSsp *);
#endif
/*----------------------------------------------------------------------------*/
static enum Result oneWireInit(void *, const void *);
static enum Result oneWireSetCallback(void *, void (*)(void *), void *);
static enum Result oneWireGetParam(void *, enum IfParameter, void *);
static enum Result oneWireSetParam(void *, enum IfParameter, const void *);
static size_t oneWireRead(void *, void *, size_t);
static size_t oneWireWrite(void *, const void *, size_t);

#ifndef CONFIG_PLATFORM_NXP_SSP_NO_DEINIT
static void oneWireDeinit(void *);
#else
#define oneWireDeinit deletedDestructorTrap
#endif
/*----------------------------------------------------------------------------*/
const struct InterfaceClass * const OneWireSsp = &(const struct InterfaceClass){
    .size = sizeof(struct OneWireSsp),
    .init = oneWireInit,
    .deinit = oneWireDeinit,

    .setCallback = oneWireSetCallback,
    .getParam = oneWireGetParam,
    .setParam = oneWireSetParam,
    .read = oneWireRead,
    .write = oneWireWrite
};
/*----------------------------------------------------------------------------*/
static void adjustPins(struct OneWireSsp *interface __attribute__((unused)),
    const struct OneWireSspConfig *config)
{
  pinSetType(pinInit(config->mosi), PIN_OPENDRAIN);
  pinSetPull(pinInit(config->mosi), PIN_PULLUP);
}
/*----------------------------------------------------------------------------*/
static void beginTransmission(struct OneWireSsp *interface)
{
  LPC_SSP_Type * const reg = interface->base.reg;

  sspSetRate(&interface->base, RATE_RESET);
  interface->state = STATE_RESET;

  /* Clear interrupt flags and enable interrupts */
  reg->ICR = ICR_RORIC | ICR_RTIC;
  reg->IMSC = IMSC_RXIM | IMSC_RTIM;

  reg->DR = PATTERN_RESET;
  reg->DR = PATTERN_PRESENCE;
}
/*----------------------------------------------------------------------------*/
static void sendWord(struct OneWireSsp *interface, uint8_t word)
{
  LPC_SSP_Type * const reg = interface->base.reg;
  uint8_t counter = 0;

  while (counter < 8)
    reg->DR = ((word >> counter++) & 0x01) ? PATTERN_HIGH : PATTERN_LOW;
}
/*----------------------------------------------------------------------------*/
static void standardInterruptHandler(void *object)
{
  struct OneWireSsp * const interface = object;
  LPC_SSP_Type * const reg = interface->base.reg;
  bool event = false;

  while (reg->SR & SR_RNE)
  {
    const uint16_t data = ~reg->DR;

    switch ((enum State)interface->state)
    {
      case STATE_RECEIVE:
        if (!(data & DATA_MASK))
          interface->word |= 1 << interface->bit;
        /* Falls through */
      case STATE_TRANSMIT:
        if (++interface->bit == 8)
        {
          if (interface->state == STATE_RECEIVE)
          {
            *interface->rxBuffer++ = interface->word;
            interface->word = 0x00;
          }
          interface->bit = 0;
          if (!--interface->left)
          {
            interface->state = STATE_IDLE;
            event = true;
          }
        }
        break;

      case STATE_RESET:
        interface->state = STATE_PRESENCE;
        break;

      case STATE_PRESENCE:
      {
        if (data & DATA_MASK)
        {
          sspSetRate(&interface->base, RATE_DATA);

          interface->bit = 0;
          interface->state = STATE_TRANSMIT;
        }
        else
        {
          interface->state = STATE_ERROR;
          event = true;
        }
        break;
      }

      default:
        break;
    }
  }

  if (reg->SR & SR_TFE)
  {
    if (interface->state == STATE_RECEIVE || interface->state == STATE_TRANSMIT)
    {
      /* Fill FIFO with next word or end the transaction */
      if (!byteQueueEmpty(&interface->txQueue))
        sendWord(interface, byteQueuePop(&interface->txQueue));
    }
  }

  if (event)
  {
    reg->IMSC = 0;

    if (interface->callback)
      interface->callback(interface->callbackArgument);
  }
}
/*----------------------------------------------------------------------------*/
#ifdef CONFIG_PLATFORM_NXP_ONE_WIRE_SSP_SEARCH
static void searchInterruptHandler(void *object)
{
  struct OneWireSsp * const interface = object;
  LPC_SSP_Type * const reg = interface->base.reg;
  bool event = false;

  while (reg->SR & SR_RNE)
  {
    const uint16_t data = ~reg->DR;

    switch ((enum State)interface->state)
    {
      case STATE_SEARCH_START:
        if (++interface->bit == 8)
        {
          interface->lastZero = 0;
          interface->left = sizeof(interface->address) * 8;
          interface->state = STATE_SEARCH_REQUEST;
        }
        break;

      case STATE_SEARCH_REQUEST:
        if (!(data & DATA_MASK))
          interface->word |= 1 << interface->bit;
        if (++interface->bit == 2)
          interface->state = STATE_SEARCH_RESPONSE;
        break;

      case STATE_SEARCH_RESPONSE:
        if (!--interface->left)
        {
          interface->lastDiscrepancy = interface->lastZero;
          interface->state = STATE_IDLE;
          event = true;
        }
        else
        {
          interface->state = STATE_SEARCH_REQUEST;
        }
        break;

      case STATE_RESET:
        interface->state = STATE_PRESENCE;
        break;

      case STATE_PRESENCE:
      {
        if (data & DATA_MASK)
        {
          sspSetRate(&interface->base, RATE_DATA);

          interface->bit = 0;
          interface->state = STATE_SEARCH_START;
        }
        else
        {
          interface->state = STATE_ERROR;
          event = true;
        }
        break;
      }

      default:
        break;
    }
  }

  if (reg->SR & SR_TFE)
  {
    switch (interface->state)
    {
      case STATE_SEARCH_START:
        sendWord(interface, 0xF0);
        break;

      case STATE_SEARCH_REQUEST:
        interface->bit = 0;
        interface->word = 0;
        sendSearchRequest(interface);
        break;

      case STATE_SEARCH_RESPONSE:
      {
        if (interface->word == 0x03)
        {
          /* No devices found */
          interface->state = STATE_ERROR;
          event = true;
          break;
        }

        const uint8_t discrepancy = interface->lastDiscrepancy;
        const uint8_t index = sizeof(interface->address) * 8 - interface->left;
        const uint64_t mask = 1ULL << index;
        bool currentBit = false;

        if (interface->word != 0x00)
        {
          currentBit = (interface->word & 0x01) != 0;
        }
        else
        {
          currentBit = (index < discrepancy && (interface->address & mask))
              || index == discrepancy;

          if (!currentBit)
            interface->lastZero = index;
        }

        if (!currentBit)
          interface->address &= ~mask;
        else
          interface->address |= mask;

        sendSearchResponse(interface, currentBit);
        break;
      }

      default:
        break;
    }
  }

  if (event)
  {
    reg->IMSC = 0;

    if (interface->callback)
      interface->callback(interface->callbackArgument);
  }
}
#endif
/*----------------------------------------------------------------------------*/
#ifdef CONFIG_PLATFORM_NXP_ONE_WIRE_SSP_SEARCH
static void sendSearchRequest(struct OneWireSsp *interface)
{
  LPC_SSP_Type * const reg = interface->base.reg;

  reg->DR = PATTERN_HIGH;
  reg->DR = PATTERN_HIGH;
}
#endif
/*----------------------------------------------------------------------------*/
#ifdef CONFIG_PLATFORM_NXP_ONE_WIRE_SSP_SEARCH
static void sendSearchResponse(struct OneWireSsp *interface, bool value)
{
  LPC_SSP_Type * const reg = interface->base.reg;

  reg->DR = value ? PATTERN_HIGH : PATTERN_LOW;
}
#endif
/*----------------------------------------------------------------------------*/
#ifdef CONFIG_PLATFORM_NXP_ONE_WIRE_SSP_SEARCH
static void startSearch(struct OneWireSsp *interface)
{
  /* Configure interrupts and start transmission */
  interface->base.handler = searchInterruptHandler;

  beginTransmission(interface);
}
#endif
/*----------------------------------------------------------------------------*/
static enum Result oneWireInit(void *object, const void *configBase)
{
  const struct OneWireSspConfig * const config = configBase;
  assert(config);

  const struct SspBaseConfig baseConfig = {
      .channel = config->channel,
      .miso = config->miso,
      .mosi = config->mosi,
      .sck = 0,
      .cs = 0
  };
  struct OneWireSsp * const interface = object;
  enum Result res;

  /* Call base class constructor */
  if ((res = SspBase->init(object, &baseConfig)) != E_OK)
    return res;

  adjustPins(interface, config);

  if ((res = byteQueueInit(&interface->txQueue, TX_QUEUE_LENGTH)) != E_OK)
    return res;

  interface->address = 0;
  interface->blocking = true;
  interface->callback = 0;
  interface->state = STATE_IDLE;

#ifdef CONFIG_PLATFORM_NXP_ONE_WIRE_SSP_SEARCH
  interface->lastDiscrepancy = 0;
#endif

  LPC_SSP_Type * const reg = interface->base.reg;

  /* Set frame size to 16 bit and SPI mode 0 */
  reg->CR0 = CR0_FRF_TI | CR0_DSS(16);
  /* Disable all interrupts */
  reg->IMSC = 0;

  sspSetRate(object, RATE_RESET);

  /* Enable peripheral */
  reg->CR1 = CR1_SSE;

  irqSetPriority(interface->base.irq, config->priority);
  irqEnable(interface->base.irq);

  return E_OK;
}
/*----------------------------------------------------------------------------*/
#ifndef CONFIG_PLATFORM_NXP_SSP_NO_DEINIT
static void oneWireDeinit(void *object)
{
  struct OneWireSsp * const interface = object;
  LPC_SSP_Type * const reg = interface->base.reg;

  /* Disable peripheral */
  irqDisable(interface->base.irq);
  reg->CR1 = 0;

  byteQueueDeinit(&interface->txQueue);
  SspBase->deinit(interface);
}
#endif
/*----------------------------------------------------------------------------*/
static enum Result oneWireSetCallback(void *object, void (*callback)(void *),
    void *argument)
{
  struct OneWireSsp * const interface = object;

  interface->callbackArgument = argument;
  interface->callback = callback;
  return E_OK;
}
/*----------------------------------------------------------------------------*/
static enum Result oneWireGetParam(void *object, enum IfParameter parameter,
    void *data __attribute__((unused)))
{
  struct OneWireSsp * const interface = object;

  switch (parameter)
  {
    case IF_ADDRESS:
      *(uint64_t *)data = fromLittleEndian64(interface->address);
      return E_OK;

    case IF_STATUS:
      if (interface->blocking || interface->state != STATE_ERROR)
        return interface->state != STATE_IDLE ? E_BUSY : E_OK;
      else
        return E_ERROR;

    default:
      return E_INVALID;
  }
}
/*----------------------------------------------------------------------------*/
static enum Result oneWireSetParam(void *object, enum IfParameter parameter,
    const void *data)
{
  struct OneWireSsp * const interface = object;

#ifdef CONFIG_PLATFORM_NXP_ONE_WIRE_SSP_SEARCH
  switch ((enum OneWireParameter)parameter)
  {
    case IF_ONE_WIRE_START_SEARCH:
      interface->address = 0;
      interface->lastDiscrepancy = 0;
      startSearch(interface);
      return E_OK;

    case IF_ONE_WIRE_FIND_NEXT:
      if (interface->lastDiscrepancy)
      {
        startSearch(interface);
        return E_OK;
      }
      else
        return E_EMPTY;

    default:
      break;
  }
#endif

  switch (parameter)
  {
    case IF_BLOCKING:
      interface->blocking = true;
      return E_OK;

    case IF_ADDRESS:
      interface->address = toLittleEndian64(*(const uint64_t *)data);
      return E_OK;

    case IF_ZEROCOPY:
      interface->blocking = false;
      return E_OK;

    default:
      return E_INVALID;
  }
}
/*----------------------------------------------------------------------------*/
static size_t oneWireRead(void *object, void *buffer, size_t length)
{
  struct OneWireSsp * const interface = object;
  LPC_SSP_Type * const reg = interface->base.reg;
  uint8_t read = 0;

  if (!length)
    return 0;

  byteQueueClear(&interface->txQueue);
  interface->bit = 0;
  interface->rxBuffer = buffer;
  interface->word = 0x00;

  /* Fill the queue with dummy words */
  while (!byteQueueFull(&interface->txQueue) && ++read != length)
    byteQueuePush(&interface->txQueue, 0xFF);
  interface->left = read;

  /* Set current mode */
  interface->state = STATE_RECEIVE;

  /* Configure interrupts */
  reg->ICR = ICR_RORIC | ICR_RTIC;
  reg->IMSC = IMSC_RXIM | IMSC_RTIM;

  /* Start reception */
  sendWord(interface, 0xFF);

  if (interface->blocking)
  {
    while (interface->state != STATE_IDLE && interface->state != STATE_ERROR)
      barrier();

    if (interface->state == STATE_ERROR)
      return 0;
  }

  return read;
}
/*----------------------------------------------------------------------------*/
static size_t oneWireWrite(void *object, const void *buffer, size_t length)
{
  struct OneWireSsp * const interface = object;

  if (!length)
    return 0;

  byteQueueClear(&interface->txQueue);
  interface->bit = 0;
  interface->left = 1;

  /* Select the addressing mode */
  if (interface->address)
  {
    byteQueuePush(&interface->txQueue, MATCH_ROM);
    interface->left += byteQueuePushArray(&interface->txQueue,
        &interface->address, sizeof(interface->address));
  }
  else
  {
    byteQueuePush(&interface->txQueue, SKIP_ROM);
  }

  /* Push data into the transmit queue */
  const size_t written = byteQueuePushArray(&interface->txQueue,
      buffer, length);
  interface->left += written;

  /* Configure interrupts and start sending */
  interface->base.handler = standardInterruptHandler;
  beginTransmission(interface);

  if (interface->blocking)
  {
    while (interface->state != STATE_IDLE && interface->state != STATE_ERROR)
      barrier();

    if (interface->state == STATE_ERROR)
      return 0;
  }

  return written;
}
