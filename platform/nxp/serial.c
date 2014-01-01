/*
 * serial.c
 * Copyright (C) 2012 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <stdbool.h>
#include <platform/nxp/serial.h>
#include <platform/nxp/uart_defs.h>
/*----------------------------------------------------------------------------*/
#define RX_FIFO_LEVEL 2 /* 8 characters */
#define TX_FIFO_SIZE  8
/*----------------------------------------------------------------------------*/
static void interruptHandler(void *);
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
const struct InterfaceClass *Serial = &serialTable;
/*----------------------------------------------------------------------------*/
static void interruptHandler(void *object)
{
  struct Serial *interface = object;
  LPC_UART_Type *reg = interface->parent.reg;
  bool event = false;

  /* Interrupt status cleared when performed read operation on IIR register */
  uint8_t state = reg->IIR;
  /* Call user handler when receive timeout occurs */
  event |= (state & IIR_INT_MASK) == IIR_INT_CTI;

  /* Byte will be removed from FIFO after reading from RBR register */
  while (reg->LSR & LSR_RDR)
  {
    uint8_t data = reg->RBR;
    /* Received bytes will be dropped when queue becomes full */
    if (!queueFull(&interface->rxQueue))
      queuePush(&interface->rxQueue, data);
  }
  if (reg->LSR & LSR_THRE)
  {
    /* Fill FIFO with selected burst size or less */
    uint32_t count = queueSize(&interface->txQueue) < TX_FIFO_SIZE
        ? queueSize(&interface->txQueue) : TX_FIFO_SIZE;

    /* Call user handler when transmit queue become half empty */
    event |= count && queueSize(&interface->txQueue) - count
        < (queueCapacity(&interface->txQueue) >> 1);

    while (count--)
      reg->THR = queuePop(&interface->txQueue);
  }

  /* User handler will be called when receive queue is half full */
  event |= queueSize(&interface->rxQueue)
      >= (queueCapacity(&interface->rxQueue) >> 1);

  if (interface->callback && event)
    interface->callback(interface->callbackArgument);
}
/*----------------------------------------------------------------------------*/
static enum result serialInit(void *object, const void *configPtr)
{
  const struct SerialConfig * const config = configPtr;
  const struct UartBaseConfig parentConfig = {
      .channel = config->channel,
      .rx = config->rx,
      .tx = config->tx
  };
  struct Serial *interface = object;
  enum result res;

  /* Call base class constructor */
  if ((res = UartBase->init(object, &parentConfig)) != E_OK)
    return res;

  interface->parent.handler = interruptHandler;

  if ((res = queueInit(&interface->rxQueue, config->rxLength)) != E_OK)
    return res;
  if ((res = queueInit(&interface->txQueue, config->txLength)) != E_OK)
    return res;

  LPC_UART_Type *reg = interface->parent.reg;

  /* Set 8-bit length */
  reg->LCR = LCR_WORD_8BIT;
  /* Enable FIFO and set RX trigger level */
  reg->FCR = (reg->FCR & ~FCR_RX_TRIGGER_MASK) | FCR_RX_TRIGGER(RX_FIFO_LEVEL)
      | FCR_ENABLE;
  /* Enable RBR and THRE interrupts */
  reg->IER = IER_RBR | IER_THRE;
  /* Enable transmitter */
  reg->TER = TER_TXEN;

  uartSetParity(object, config->parity);
  uartSetRate(object, uartCalcRate(object, config->rate));

  irqSetPriority(interface->parent.irq, config->priority);
  irqEnable(interface->parent.irq);

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static void serialDeinit(void *object)
{
  struct Serial *interface = object;

  irqDisable(interface->parent.irq);
  queueDeinit(&interface->txQueue);
  queueDeinit(&interface->rxQueue);
  UartBase->deinit(interface);
}
/*----------------------------------------------------------------------------*/
static enum result serialCallback(void *object, void (*callback)(void *),
    void *argument)
{
  struct Serial *interface = object;

  interface->callbackArgument = argument;
  interface->callback = callback;
  return E_OK;
}
/*----------------------------------------------------------------------------*/
static enum result serialGet(void *object, enum ifOption option, void *data)
{
  struct Serial *interface = object;

  switch (option)
  {
    case IF_AVAILABLE:
      *(uint32_t *)data = queueSize(&interface->rxQueue);
      return E_OK;
    case IF_PENDING:
      *(uint32_t *)data = queueSize(&interface->txQueue);
      return E_OK;
    default:
      return E_ERROR;
  }
}
/*----------------------------------------------------------------------------*/
static enum result serialSet(void *object, enum ifOption option,
    const void *data)
{
  switch (option)
  {
    case IF_RATE:
      uartSetRate(object, uartCalcRate(object, *(uint32_t *)data));
      return E_OK;
    default:
      return E_ERROR;
  }
}
/*----------------------------------------------------------------------------*/
static uint32_t serialRead(void *object, uint8_t *buffer, uint32_t length)
{
  struct Serial *interface = object;
  uint32_t read;

  irqDisable(interface->parent.irq);
  read = queuePopArray(&interface->rxQueue, buffer, length);
  irqEnable(interface->parent.irq);

  return read;
}
/*----------------------------------------------------------------------------*/
static uint32_t serialWrite(void *object, const uint8_t *buffer,
    uint32_t length)
{
  struct Serial *interface = object;
  LPC_UART_Type *reg = interface->parent.reg;
  uint32_t written = 0;

  irqDisable(interface->parent.irq);
  /* Check transmitter state */
  if (reg->LSR & LSR_TEMT && queueEmpty(&interface->txQueue))
  {
    /* Transmitter is idle so fill TX FIFO */
    uint32_t count = length < TX_FIFO_SIZE ? length : TX_FIFO_SIZE;
    for (; written < count; ++written)
      reg->THR = *buffer++;
    length -= written;
  }
  /* Fill TX queue with the rest of data */
  if (length)
    written += queuePushArray(&interface->txQueue, buffer, length);
  irqEnable(interface->parent.irq);

  return written;
}
