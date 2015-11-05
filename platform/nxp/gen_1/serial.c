/*
 * serial.c
 * Copyright (C) 2012 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <stdbool.h>
#include <pm.h>
#include <platform/nxp/serial.h>
#include <platform/nxp/gen_1/uart_defs.h>
/*----------------------------------------------------------------------------*/
#define TX_FIFO_SIZE 8
/*----------------------------------------------------------------------------*/
static void interruptHandler(void *);
/*----------------------------------------------------------------------------*/
#ifdef CONFIG_SERIAL_PM
static enum result powerStateHandler(void *, enum pmState);
#endif
/*----------------------------------------------------------------------------*/
static enum result serialInit(void *, const void *);
static void serialDeinit(void *);
static enum result serialCallback(void *, void (*)(void *), void *);
static enum result serialGet(void *, enum ifOption, void *);
static enum result serialSet(void *, enum ifOption, const void *);
static uint32_t serialRead(void *, uint8_t *, uint32_t);
static uint32_t serialWrite(void *, const uint8_t *, uint32_t);
/*----------------------------------------------------------------------------*/
static const struct InterfaceClass serialTable = {
    .size = sizeof(struct Serial),
    .init = serialInit,
    .deinit = serialDeinit,

    .callback = serialCallback,
    .get = serialGet,
    .set = serialSet,
    .read = serialRead,
    .write = serialWrite
};
/*----------------------------------------------------------------------------*/
const struct InterfaceClass * const Serial = &serialTable;
/*----------------------------------------------------------------------------*/
static void interruptHandler(void *object)
{
  struct Serial * const interface = object;
  LPC_UART_Type * const reg = interface->parent.reg;
  bool event = false;

  /* Interrupt status cleared when performed read operation on IIR register */
  const uint32_t state = reg->IIR;
  /* Call user handler when receive timeout occurs */
  event |= (state & IIR_INT_MASK) == IIR_INT_CTI;

  /* Byte will be removed from FIFO after reading from RBR register */
  while (reg->LSR & LSR_RDR)
  {
    const uint8_t data = (uint8_t)reg->RBR;

    /* Received bytes will be dropped when queue becomes full */
    if (!byteQueueFull(&interface->rxQueue))
      byteQueuePush(&interface->rxQueue, data);
  }
  if (reg->LSR & LSR_THRE)
  {
    const uint32_t txQueueCapacity = byteQueueCapacity(&interface->txQueue);
    const uint32_t txQueueSize = byteQueueSize(&interface->txQueue);

    /* Fill FIFO with selected burst size or less */
    uint32_t count = txQueueSize < TX_FIFO_SIZE ? txQueueSize : TX_FIFO_SIZE;

    /* Call user handler when transmit queue becomes half empty */
    event |= count && txQueueSize - count < (txQueueCapacity >> 1);

    while (count--)
      reg->THR = byteQueuePop(&interface->txQueue);
  }

  /* User handler will be called when receive queue becomes half full */
  event |= byteQueueSize(&interface->rxQueue) >=
      (byteQueueCapacity(&interface->rxQueue) >> 1);

  if (interface->callback && event)
    interface->callback(interface->callbackArgument);
}
/*----------------------------------------------------------------------------*/
#ifdef CONFIG_SERIAL_PM
static enum result powerStateHandler(void *object, enum pmState state)
{
  struct Serial * const interface = object;
  struct UartRateConfig rateConfig;
  enum result res;

  if (state == PM_ACTIVE)
  {
    /* Recalculate and set baud rate */
    if ((res = uartCalcRate(object, interface->rate, &rateConfig)) != E_OK)
      return res;

    uartSetRate(object, rateConfig);
  }

  return E_OK;
}
#endif
/*----------------------------------------------------------------------------*/
static enum result serialInit(void *object, const void *configBase)
{
  const struct SerialConfig * const config = configBase;
  const struct UartBaseConfig parentConfig = {
      .channel = config->channel,
      .rx = config->rx,
      .tx = config->tx
  };
  struct Serial * const interface = object;
  struct UartRateConfig rateConfig;
  enum result res;

  /* Call base class constructor */
  if ((res = UartBase->init(object, &parentConfig)) != E_OK)
    return res;

  if ((res = uartCalcRate(object, config->rate, &rateConfig)) != E_OK)
    return res;

  interface->parent.handler = interruptHandler;

  interface->callback = 0;
  interface->rate = config->rate;

  if ((res = byteQueueInit(&interface->rxQueue, config->rxLength)) != E_OK)
    return res;
  if ((res = byteQueueInit(&interface->txQueue, config->txLength)) != E_OK)
    return res;

  LPC_UART_Type * const reg = interface->parent.reg;

  /* Set 8-bit length */
  reg->LCR = LCR_WORD_8BIT;
  /* Enable FIFO and set RX trigger level */
  reg->FCR = (reg->FCR & ~FCR_RX_TRIGGER_MASK) | FCR_ENABLE
      | FCR_RX_TRIGGER(RX_TRIGGER_LEVEL_8);
  /* Enable RBR and THRE interrupts */
  reg->IER = IER_RBR | IER_THRE;
  /* Transmitter is enabled by default thus TER register is left untouched */

  uartSetParity(object, config->parity);
  uartSetRate(object, rateConfig);

#ifdef CONFIG_SERIAL_PM
  if ((res = pmRegister(object, powerStateHandler)) != E_OK)
    return res;
#endif

  irqSetPriority(interface->parent.irq, config->priority);
  irqEnable(interface->parent.irq);

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static void serialDeinit(void *object)
{
  struct Serial * const interface = object;

  irqDisable(interface->parent.irq);
  byteQueueDeinit(&interface->txQueue);
  byteQueueDeinit(&interface->rxQueue);
  UartBase->deinit(interface);
}
/*----------------------------------------------------------------------------*/
static enum result serialCallback(void *object, void (*callback)(void *),
    void *argument)
{
  struct Serial * const interface = object;

  interface->callbackArgument = argument;
  interface->callback = callback;
  return E_OK;
}
/*----------------------------------------------------------------------------*/
static enum result serialGet(void *object, enum ifOption option, void *data)
{
  struct Serial * const interface = object;

  switch (option)
  {
    case IF_AVAILABLE:
      *(uint32_t *)data = byteQueueSize(&interface->rxQueue);
      return E_OK;

    case IF_PENDING:
      *(uint32_t *)data = byteQueueSize(&interface->txQueue);
      return E_OK;

    default:
      return E_ERROR;
  }
}
/*----------------------------------------------------------------------------*/
static enum result serialSet(void *object, enum ifOption option,
    const void *data)
{
  struct Serial * const interface = object;
  struct UartRateConfig rateConfig;
  enum result res;

  switch (option)
  {
    case IF_RATE:
      res = uartCalcRate(object, *(const uint32_t *)data, &rateConfig);

      if (res == E_OK)
      {
        interface->rate = *(const uint32_t *)data;
        uartSetRate(object, rateConfig);
      }
      return res;

    default:
      return E_ERROR;
  }
}
/*----------------------------------------------------------------------------*/
static uint32_t serialRead(void *object, uint8_t *buffer, uint32_t length)
{
  struct Serial * const interface = object;
  uint32_t read;

  irqDisable(interface->parent.irq);
  read = byteQueuePopArray(&interface->rxQueue, buffer, length);
  irqEnable(interface->parent.irq);

  return read;
}
/*----------------------------------------------------------------------------*/
static uint32_t serialWrite(void *object, const uint8_t *buffer,
    uint32_t length)
{
  struct Serial * const interface = object;
  LPC_UART_Type * const reg = interface->parent.reg;
  uint32_t written = 0;

  irqDisable(interface->parent.irq);
  /* Check transmitter state */
  if (reg->LSR & LSR_THRE && byteQueueEmpty(&interface->txQueue))
  {
    /* Transmitter is idle so fill TX FIFO */
    const uint32_t count = length < TX_FIFO_SIZE ? length : TX_FIFO_SIZE;

    for (; written < count; ++written)
      reg->THR = *buffer++;
    length -= written;
  }
  /* Fill TX queue with the rest of data */
  if (length)
    written += byteQueuePushArray(&interface->txQueue, buffer, length);
  irqEnable(interface->parent.irq);

  return written;
}
