/*
 * one_wire.c
 * Copyright (C) 2013 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <assert.h>
#include "platform/nxp/one_wire.h"
#include "platform/nxp/uart_defs.h"
/*----------------------------------------------------------------------------*/
#define DEFAULT_PRIORITY    255 /* Lowest interrupt priority in Cortex-M3 */
#define RATE_RESET          9600
#define RATE_DATA           115200
/*----------------------------------------------------------------------------*/
enum romCommand
{
  SEARCH_ROM  = 0xF0,
  READ_ROM    = 0x33,
  MATCH_ROM   = 0x55,
  SKIP_ROM    = 0xCC
};
/*----------------------------------------------------------------------------*/
static void beginTransmission(struct OneWire *);
static void interruptHandler(void *);
static void sendWord(struct OneWire *, uint8_t);
/*----------------------------------------------------------------------------*/
static enum result oneWireInit(void *, const void *);
static void oneWireDeinit(void *);
static enum result oneWireCallback(void *, void (*)(void *), void *);
static enum result oneWireGet(void *, enum ifOption, void *);
static enum result oneWireSet(void *, enum ifOption, const void *);
static uint32_t oneWireRead(void *, uint8_t *, uint32_t);
static uint32_t oneWireWrite(void *, const uint8_t *, uint32_t);
/*----------------------------------------------------------------------------*/
static const struct InterfaceClass serialTable = {
    .size = sizeof(struct OneWire),
    .init = oneWireInit,
    .deinit = oneWireDeinit,

    .callback = oneWireCallback,
    .get = oneWireGet,
    .set = oneWireSet,
    .read = oneWireRead,
    .write = oneWireWrite
};
/*----------------------------------------------------------------------------*/
const struct InterfaceClass *OneWire = &serialTable;
/*----------------------------------------------------------------------------*/
static void beginTransmission(struct OneWire *interface)
{
  LPC_UART_TypeDef *reg = interface->parent.reg;

  uartSetRate((struct Uart *)interface, interface->resetRate);
  interface->state = OW_RESET;
  /* Clear RX FIFO and set trigger level to 1 character */
  reg->FCR |= FCR_RX_RESET | FCR_RX_TRIGGER(0);
  reg->THR = 0xF0; /* Execute reset */
}
/*----------------------------------------------------------------------------*/
static void sendWord(struct OneWire *interface, uint8_t word)
{
  LPC_UART_TypeDef *reg = interface->parent.reg;
  uint8_t counter = 0;

  while (counter < 8)
    reg->THR = (word >> counter++) & 0x01 ? 0xFF : 0;
/*    reg->THR = ((word >> counter++) & 0x01) * 0xFF; FIXME */
}
/*----------------------------------------------------------------------------*/
static void interruptHandler(void *object)
{
  struct OneWire *interface = object;
  LPC_UART_TypeDef *reg = interface->parent.reg;

  /* Interrupt status cleared when performed read operation on IIR register */
  if (reg->IIR & IIR_INT_STATUS)
    return;

  /* Byte will be removed from FIFO after reading from RBR register */
  while (reg->LSR & LSR_RDR)
  {
    uint8_t data = reg->RBR;
    switch (interface->state)
    {
      case OW_RESET:
        if (data ^ 0xF0)
        {
          uartSetRate((struct Uart *)interface, interface->dataRate);
          interface->state = OW_TRANSMIT;
        }
        else
          interface->state = OW_IDLE;
        break;
      case OW_RECEIVE:
        if (data & 0x01)
          interface->word |= 1 << interface->rxPosition;
      case OW_TRANSMIT:
        if (++interface->rxPosition == 8)
        {
          if (interface->state == OW_RECEIVE)
          {
            *interface->rxBuffer++ = interface->word;
            interface->word = 0x00;
          }
          interface->rxPosition = 0;
          if (!--interface->left)
          {
            interface->state = OW_IDLE;
            if (interface->callback)
              interface->callback(interface->callbackArgument);
          }
        }
        break;
      default:
        break;
    }
  }
  if ((reg->LSR & LSR_THRE) && interface->state != OW_RESET)
  {
    /* Fill FIFO with next word or end the transaction */
    if (!queueEmpty(&interface->txQueue))
      sendWord(interface, queuePop(&interface->txQueue));
  }
}
/*----------------------------------------------------------------------------*/
static enum result oneWireInit(void *object, const void *configPtr)
{
  /* Set pointer to interface configuration data */
  const struct OneWireConfig * const config = configPtr;
  struct OneWire *interface = object;
  struct UartConfig parentConfig;
  enum result res;

  /* Check interface configuration data */
  assert(config);

  /* Compute rates */
   if ((res = uartCalcRate(&interface->dataRate, RATE_DATA)) != E_OK)
     return res;
   if ((res = uartCalcRate(&interface->resetRate, RATE_RESET)) != E_OK)
     return res;

  /* Initialize parent configuration structure */
  parentConfig.channel = config->channel;
  parentConfig.rx = config->rx;
  parentConfig.tx = config->tx;
  parentConfig.rate = RATE_RESET;

  /* Call UART class constructor */
  if ((res = Uart->init(object, &parentConfig)) != E_OK)
    return res;

  gpioSetType(&interface->parent.txPin, GPIO_OPENDRAIN);

  /* Set pointer to hardware interrupt handler */
  interface->parent.handler = interruptHandler;

  /* Initialize TX queue */
  if ((res = queueInit(&interface->txQueue, config->txLength)) != E_OK)
    return res;
  /* Create mutex */
  if ((res = mutexInit(&interface->channelLock)) != E_OK)
    return res;

  interface->callback = 0;

  interface->blocking = true;
  interface->state = OW_IDLE;
  interface->address.rom = 0;

  /* Initialize UART block */
  LPC_UART_TypeDef *reg = interface->parent.reg;

  /* Set RX trigger level to single byte */
  reg->FCR &= ~FCR_RX_TRIGGER_MASK;
  /* Enable RBR and THRE interrupts */
  reg->IER |= IER_RBR | IER_THRE;

  /* Set interrupt priority, lowest by default */
  nvicSetPriority(interface->parent.irq, DEFAULT_PRIORITY);
  /* Enable UART interrupt */
  nvicEnable(interface->parent.irq);

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static void oneWireDeinit(void *object)
{
  struct OneWire *interface = object;

  nvicDisable(interface->parent.irq); /* Disable interrupt */
  mutexDeinit(&interface->channelLock);
  queueDeinit(&interface->txQueue);
  Uart->deinit(interface); /* Call UART class destructor */
}
/*----------------------------------------------------------------------------*/
static enum result oneWireCallback(void *object, void (*callback)(void *),
    void *argument)
{
  struct OneWire *interface = object;

  interface->callback = callback;
  interface->callbackArgument = argument;
  return E_OK;
}
/*----------------------------------------------------------------------------*/
static enum result oneWireGet(void *object, enum ifOption option, void *data)
{
  struct OneWire *interface = object;

  switch (option)
  {
    case IF_BUSY:
      *(uint32_t *)data = interface->state != OW_IDLE;
      return E_OK;
    case IF_DEVICE:
      *(uint64_t *)data = interface->address.rom;
      return E_OK;
    case IF_PRIORITY:
      *(uint32_t *)data = nvicGetPriority(interface->parent.irq);
      return E_OK;
    case IF_ZEROCOPY:
      *(uint32_t *)data = !interface->blocking;
      return E_OK;
    default:
      return E_ERROR;
  }
}
/*----------------------------------------------------------------------------*/
static enum result oneWireSet(void *object, enum ifOption option,
    const void *data)
{
  struct OneWire *interface = object;

  switch (option)
  {
    case IF_DEVICE:
      interface->address.rom = *(uint64_t *)data;
      return E_OK;
    case IF_LOCK:
      if (*(uint32_t *)data)
      {
        if (interface->blocking)
          return mutexTryLock(&interface->channelLock) ? E_OK : E_BUSY;
        else
          mutexLock(&interface->channelLock);
      }
      else
      {
        mutexUnlock(&interface->channelLock);
      }
      return E_OK;
    case IF_PRIORITY:
      nvicSetPriority(interface->parent.irq, *(uint32_t *)data);
      return E_OK;
    case IF_ZEROCOPY:
      interface->blocking = *(uint32_t *)data ? false : true;
      return E_OK;
    default:
      return E_ERROR;
  }
}
/*----------------------------------------------------------------------------*/
static uint32_t oneWireRead(void *object, uint8_t *buffer, uint32_t length)
{
  struct OneWire *interface = object;
  uint32_t read = 0;

  if (!length)
    return 0;

  queueClear(&interface->txQueue);
  interface->rxPosition = 0;
  interface->rxBuffer = buffer;
  interface->word = 0x00;
  while (!queueFull(&interface->txQueue) && ++read != length)
    queuePush(&interface->txQueue, 0xFF);
  interface->left = read;

  interface->state = OW_RECEIVE;
  /* Clear RX FIFO and set trigger level to 8 characters */
  ((LPC_UART_TypeDef *)interface->parent.reg)->FCR |= FCR_RX_RESET
      | FCR_RX_TRIGGER(2);
  sendWord(interface, 0xFF); /* Start reception */

  if (interface->blocking)
    while (interface->state != OW_IDLE);

  return read;
}
/*----------------------------------------------------------------------------*/
static uint32_t oneWireWrite(void *object, const uint8_t *buffer,
    uint32_t length)
{
  struct OneWire *interface = object;
  uint32_t written;

  if (!length)
    return 0;

  queueClear(&interface->txQueue);
  interface->rxPosition = 0;
  interface->left = 1;
  /* Initiate new transaction by selecting addressing mode */
  if (interface->address.rom)
  {
    queuePush(&interface->txQueue, (uint8_t)MATCH_ROM);
    interface->left += queuePushArray(&interface->txQueue,
        (const uint8_t *)&interface->address.rom, length);
  }
  else
    queuePush(&interface->txQueue, (uint8_t)SKIP_ROM);
  written = queuePushArray(&interface->txQueue, buffer, length);
  interface->left += written;

  beginTransmission(interface);

  if (interface->blocking)
    while (interface->state != OW_IDLE);

  return written;
}
