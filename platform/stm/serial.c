/*
 * serial.c
 * Copyright (C) 2016 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <stdbool.h>
#include <halm/platform/stm/serial.h>
#include <halm/platform/stm/uart_defs.h>
/*----------------------------------------------------------------------------*/
static void interruptHandler(void *);
/*----------------------------------------------------------------------------*/
static enum result serialInit(void *, const void *);
static void serialDeinit(void *);
static enum result serialCallback(void *, void (*)(void *), void *);
static enum result serialGet(void *, enum ifOption, void *);
static enum result serialSet(void *, enum ifOption, const void *);
static size_t serialRead(void *, void *, size_t);
static size_t serialWrite(void *, const void *, size_t);
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
  STM_USART_Type * const reg = interface->base.reg;
  bool event;

  const uint32_t status = reg->SR;

  /* Handle received data */
  if (status & SR_RXNE)
  {
    const uint8_t data = reg->DR;

    /* Received bytes will be dropped when queue becomes full */
    if (!byteQueueFull(&interface->rxQueue))
      byteQueuePush(&interface->rxQueue, data);
  }

  /* Handle reception timeout */
  if (status & SR_IDLE)
  {
    (void)reg->DR; /* Clear IDLE flag */
    event = true;
  }

  const uint32_t control = reg->CR1;

  /* Send queued data */
  if ((control & CR1_TXEIE) && (status & SR_TXE))
  {
    if (!byteQueueEmpty(&interface->txQueue))
      reg->DR = byteQueuePop(&interface->txQueue);

    if (byteQueueEmpty(&interface->txQueue))
      reg->CR1 = control & ~CR1_TXEIE;
  }

  /* User handler will be called when receive queue is half-full */
  const size_t rxQueueLevel = byteQueueCapacity(&interface->rxQueue) / 2;
  const size_t rxQueueSize = byteQueueSize(&interface->rxQueue);

  event = event || rxQueueSize >= rxQueueLevel;

  if (interface->callback && event)
    interface->callback(interface->callbackArgument);
}
/*----------------------------------------------------------------------------*/
static enum result serialInit(void *object, const void *configBase)
{
  const struct SerialConfig * const config = configBase;
  const struct UartBaseConfig baseConfig = {
      .channel = config->channel,
      .rx = config->rx,
      .tx = config->tx
  };
  struct Serial * const interface = object;
  enum result res;

  /* Call base class constructor */
  if ((res = UartBase->init(object, &baseConfig)) != E_OK)
    return res;

  interface->base.handler = interruptHandler;

  interface->callback = 0;
  interface->rate = config->rate;

  if ((res = byteQueueInit(&interface->rxQueue, config->rxLength)) != E_OK)
    return res;
  if ((res = byteQueueInit(&interface->txQueue, config->txLength)) != E_OK)
    return res;

  uartSetRate(object, config->rate);
  uartSetParity(object, config->parity);

  STM_USART_Type * const reg = interface->base.reg;

  /*
   * Enable receiver and transmitter, RXNE and IDLE interrupts,
   * enable peripheral.
   */
  reg->CR1 = CR1_RE | CR1_TE | CR1_RXNEIE | CR1_IDLEIE | CR1_UE;

  irqSetPriority(interface->base.irq, config->priority);
  irqEnable(interface->base.irq);

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static void serialDeinit(void *object)
{
  struct Serial * const interface = object;
  STM_USART_Type * const reg = interface->base.reg;

  irqDisable(interface->base.irq);
  reg->CR1 = 0;

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
      *(size_t *)data = byteQueueSize(&interface->rxQueue);
      return E_OK;

    case IF_PENDING:
      *(size_t *)data = byteQueueSize(&interface->txQueue);
      return E_OK;

    default:
      return E_INVALID;
  }
}
/*----------------------------------------------------------------------------*/
static enum result serialSet(void *object, enum ifOption option,
    const void *data)
{
  struct Serial * const interface = object;

  switch (option)
  {
    case IF_RATE:
      interface->rate = *(const uint32_t *)data;
      uartSetRate(object, interface->rate);
      return E_OK;

    default:
      return E_INVALID;
  }
}
/*----------------------------------------------------------------------------*/
static size_t serialRead(void *object, void *buffer, size_t length)
{
  struct Serial * const interface = object;
  size_t read;

  irqDisable(interface->base.irq);
  read = byteQueuePopArray(&interface->rxQueue, buffer, length);
  irqEnable(interface->base.irq);

  return read;
}
/*----------------------------------------------------------------------------*/
static size_t serialWrite(void *object, const void *buffer, size_t length)
{
  struct Serial * const interface = object;
  STM_USART_Type * const reg = interface->base.reg;
  size_t written;

  if (length)
  {
    irqDisable(interface->base.irq);
    written = byteQueuePushArray(&interface->txQueue, buffer, length);
    if (!(reg->CR1 & CR1_TXEIE))
      reg->CR1 |= CR1_TXEIE;
    irqEnable(interface->base.irq);
  }
  else
    written = 0;

  return written;
}
