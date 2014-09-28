/*
 * one_wire_ssp.c
 * Copyright (C) 2014 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <memory.h>
#include <platform/nxp/one_wire_ssp.h>
#include <platform/nxp/ssp_defs.h>
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
enum oneWireRomCommand
{
  SEARCH_ROM  = 0xF0,
  READ_ROM    = 0x33,
  MATCH_ROM   = 0x55,
  SKIP_ROM    = 0xCC
};
/*----------------------------------------------------------------------------*/
static void adjustPins(struct OneWireSsp *, const struct OneWireSspConfig *);
static void beginTransmission(struct OneWireSsp *);
static void interruptHandler(void *);
static void sendWord(struct OneWireSsp *, uint8_t);
/*----------------------------------------------------------------------------*/
static enum result oneWireInit(void *, const void *);
static void oneWireDeinit(void *);
static enum result oneWireCallback(void *, void (*)(void *), void *);
static enum result oneWireGet(void *, enum ifOption, void *);
static enum result oneWireSet(void *, enum ifOption, const void *);
static uint32_t oneWireRead(void *, uint8_t *, uint32_t);
static uint32_t oneWireWrite(void *, const uint8_t *, uint32_t);
/*----------------------------------------------------------------------------*/
static const struct InterfaceClass oneWireTable = {
    .size = sizeof(struct OneWireSsp),
    .init = oneWireInit,
    .deinit = oneWireDeinit,

    .callback = oneWireCallback,
    .get = oneWireGet,
    .set = oneWireSet,
    .read = oneWireRead,
    .write = oneWireWrite
};
/*----------------------------------------------------------------------------*/
const struct InterfaceClass * const OneWireSsp = &oneWireTable;
/*----------------------------------------------------------------------------*/
static void adjustPins(struct OneWireSsp *interface __attribute__((unused)),
    const struct OneWireSspConfig *config)
{
  pinSetType(pinInit(config->mosi), PIN_OPENDRAIN);
}
/*----------------------------------------------------------------------------*/
static void beginTransmission(struct OneWireSsp *interface)
{
  LPC_SSP_Type * const reg = interface->parent.reg;

  sspSetRate((struct SspBase *)interface, RATE_RESET);
  interface->state = OW_SSP_RESET;
  reg->DR = PATTERN_RESET;
  reg->DR = PATTERN_PRESENCE;
}
/*----------------------------------------------------------------------------*/
static void sendWord(struct OneWireSsp *interface, uint8_t word)
{
  LPC_SSP_Type * const reg = interface->parent.reg;
  uint8_t counter = 0;

  while (counter < 8)
    reg->DR = (word >> counter++) & 0x01 ? PATTERN_HIGH : PATTERN_LOW;
}
/*----------------------------------------------------------------------------*/
static void interruptHandler(void *object)
{
  struct OneWireSsp * const interface = object;
  LPC_SSP_Type * const reg = interface->parent.reg;
  bool event = false;

  while (reg->SR & SR_RNE)
  {
    const uint16_t data = ~reg->DR;

    switch (interface->state)
    {
      case OW_SSP_RECEIVE:
        if (!(data & DATA_MASK))
          interface->word |= 1 << interface->bit;
      case OW_SSP_TRANSMIT:
        if (++interface->bit == 8)
        {
          if (interface->state == OW_SSP_RECEIVE)
          {
            *interface->rxBuffer++ = interface->word;
            interface->word = 0x00;
          }
          interface->bit = 0;
          if (!--interface->left)
          {
            interface->state = OW_SSP_IDLE;
            event = true;
          }
        }
        break;

      case OW_SSP_RESET:
        interface->state = OW_SSP_PRESENCE;
        break;

      case OW_SSP_PRESENCE:
        if (data & DATA_MASK)
        {
          sspSetRate((struct SspBase *)object, RATE_DATA);
          interface->state = OW_SSP_TRANSMIT;
        }
        else
        {
          interface->state = OW_SSP_ERROR;
          event = true;
        }
        break;

      default:
        break;
    }
  }

  if ((reg->SR & SR_TFE) && (interface->state == OW_SSP_RECEIVE
      || interface->state == OW_SSP_TRANSMIT))
  {
    /* Fill FIFO with next word or end the transaction */
    if (!byteQueueEmpty(&interface->txQueue))
      sendWord(interface, byteQueuePop(&interface->txQueue));
  }

  if (event)
  {
    reg->IMSC &= ~(IMSC_RXIM | IMSC_RTIM);

    if (interface->callback)
      interface->callback(interface->callbackArgument);
  }
}
/*----------------------------------------------------------------------------*/
static enum result oneWireInit(void *object, const void *configBase)
{
  const struct OneWireSspConfig * const config = configBase;
  const struct SspBaseConfig parentConfig = {
      .channel = config->channel,
      .miso = config->miso,
      .mosi = config->mosi,
      .sck = 0,
      .cs = 0
  };
  struct OneWireSsp * const interface = object;
  enum result res;

  /* Call base class constructor */
  if ((res = SspBase->init(object, &parentConfig)) != E_OK)
    return res;

  adjustPins(interface, config);

  if ((res = byteQueueInit(&interface->txQueue, TX_QUEUE_LENGTH)) != E_OK)
    return res;

  interface->parent.handler = interruptHandler;

  interface->address = 0;
  interface->blocking = true;
  interface->callback = 0;
  interface->state = OW_SSP_IDLE;

  LPC_SSP_Type * const reg = interface->parent.reg;

  /* Set frame size to 16 bit and SPI mode 0 */
  reg->CR0 = CR0_FRF_TI | CR0_DSS(16);

  sspSetRate(object, RATE_RESET);
  /* Enable peripheral */
  reg->CR1 = CR1_SSE;

  irqSetPriority(interface->parent.irq, config->priority);
  irqEnable(interface->parent.irq);

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static void oneWireDeinit(void *object)
{
  struct OneWireSsp * const interface = object;

  irqDisable(interface->parent.irq);
  byteQueueDeinit(&interface->txQueue);
  SspBase->deinit(interface);
}
/*----------------------------------------------------------------------------*/
static enum result oneWireCallback(void *object, void (*callback)(void *),
    void *argument)
{
  struct OneWireSsp * const interface = object;

  interface->callbackArgument = argument;
  interface->callback = callback;
  return E_OK;
}
/*----------------------------------------------------------------------------*/
static enum result oneWireGet(void *object, enum ifOption option,
    void *data __attribute__((unused)))
{
  struct OneWireSsp * const interface = object;

  switch (option)
  {
    case IF_STATUS:
      if (!interface->blocking && interface->state == OW_SSP_ERROR)
        return E_ERROR;
      else
        return interface->state != OW_SSP_IDLE ? E_BUSY : E_OK;

    default:
      return E_ERROR;
  }
}
/*----------------------------------------------------------------------------*/
static enum result oneWireSet(void *object, enum ifOption option,
    const void *data)
{
  struct OneWireSsp * const interface = object;

  switch (option)
  {
    case IF_BLOCKING:
      interface->blocking = true;
      return E_OK;

    case IF_DEVICE:
      interface->address = toLittleEndian64(*(const uint64_t *)data);
      return E_OK;

    case IF_ZEROCOPY:
      interface->blocking = false;
      return E_OK;

    default:
      return E_ERROR;
  }
}
/*----------------------------------------------------------------------------*/
static uint32_t oneWireRead(void *object, uint8_t *buffer, uint32_t length)
{
  struct OneWireSsp * const interface = object;
  LPC_SSP_Type * const reg = interface->parent.reg;
  uint32_t read = 0;

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

  interface->state = OW_SSP_RECEIVE;

  /* Clear interrupt flags, enable interrupts and start reception */
  reg->ICR = ICR_RORIC | ICR_RTIC;
  reg->IMSC |= IMSC_RXIM | IMSC_RTIM;
  sendWord(interface, 0xFF);

  if (interface->blocking)
  {
    while (interface->state != OW_SSP_IDLE && interface->state != OW_SSP_ERROR)
      barrier();

    if (interface->state == OW_SSP_ERROR)
      return 0;
  }

  return read;
}
/*----------------------------------------------------------------------------*/
static uint32_t oneWireWrite(void *object, const uint8_t *buffer,
    uint32_t length)
{
  struct OneWireSsp * const interface = object;
  LPC_SSP_Type * const reg = interface->parent.reg;
  uint32_t written;

  if (!length)
    return 0;

  byteQueueClear(&interface->txQueue);
  interface->bit = 0;
  interface->left = 1;
  /* Initiate new transaction by selecting the addressing mode */
  if (interface->address)
  {
    byteQueuePush(&interface->txQueue, (uint8_t)MATCH_ROM);
    interface->left += byteQueuePushArray(&interface->txQueue,
        (const uint8_t *)&interface->address, length);
  }
  else
    byteQueuePush(&interface->txQueue, (uint8_t)SKIP_ROM);
  written = byteQueuePushArray(&interface->txQueue, buffer, length);
  interface->left += written;

  /* Clear interrupt flags, enable interrupts and start transmission */
  reg->ICR = ICR_RORIC | ICR_RTIC;
  reg->IMSC |= IMSC_RXIM | IMSC_RTIM;
  beginTransmission(interface);

  if (interface->blocking)
  {
    while (interface->state != OW_SSP_IDLE && interface->state != OW_SSP_ERROR)
      barrier();

    if (interface->state == OW_SSP_ERROR)
      return 0;
  }

  return written;
}
