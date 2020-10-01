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
static enum Result serialInit(void *, const void *);
static void serialSetCallback(void *, void (*)(void *), void *);
static enum Result serialGetParam(void *, enum IfParameter, void *);
static enum Result serialSetParam(void *, enum IfParameter, const void *);
static size_t serialRead(void *, void *, size_t);
static size_t serialWrite(void *, const void *, size_t);

#ifndef CONFIG_PLATFORM_STM_UART_NO_DEINIT
static void serialDeinit(void *);
#else
#define serialDeinit deletedDestructorTrap
#endif
/*----------------------------------------------------------------------------*/
const struct InterfaceClass * const Serial = &(const struct InterfaceClass){
    .size = sizeof(struct Serial),
    .init = serialInit,
    .deinit = serialDeinit,

    .setCallback = serialSetCallback,
    .getParam = serialGetParam,
    .setParam = serialSetParam,
    .read = serialRead,
    .write = serialWrite
};
/*----------------------------------------------------------------------------*/
static void interruptHandler(void *object)
{
  struct Serial * const interface = object;
  STM_USART_Type * const reg = interface->base.reg;
  bool event = false;

  const uint32_t status = reg->SR;

  /* Handle received data */
  if (status & SR_RXNE)
  {
    const uint8_t data = reg->DR;

    /* Received bytes will be dropped when queue becomes full */
    if (!byteQueueFull(&interface->rxQueue))
      byteQueuePushBack(&interface->rxQueue, data);
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
      reg->DR = byteQueuePopFront(&interface->txQueue);

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
static enum Result serialInit(void *object, const void *configBase)
{
  const struct SerialConfig * const config = configBase;
  assert(config);

  const struct UartBaseConfig baseConfig = {
      .rx = config->rx,
      .tx = config->tx,
      .channel = config->channel
  };
  struct Serial * const interface = object;
  enum Result res;

  /* Call base class constructor */
  if ((res = UartBase->init(object, &baseConfig)) != E_OK)
    return res;

  interface->base.handler = interruptHandler;

  interface->callback = 0;
  interface->rate = config->rate;

  if (!byteQueueInit(&interface->rxQueue, config->rxLength))
    return E_MEMORY;
  if (!byteQueueInit(&interface->txQueue, config->txLength))
    return E_MEMORY;

  STM_USART_Type * const reg = interface->base.reg;

  /* Disable the peripheral */
  reg->CR1 = 0;

  uartSetRate(object, config->rate);
  uartSetParity(object, config->parity);

  /*
   * Enable receiver and transmitter, RXNE and IDLE interrupts,
   * enable peripheral.
   */
  reg->CR1 |= CR1_RE | CR1_TE | CR1_RXNEIE | CR1_IDLEIE | CR1_UE;

  irqSetPriority(interface->base.irq, config->priority);
  irqEnable(interface->base.irq);

  return E_OK;
}
/*----------------------------------------------------------------------------*/
#ifndef CONFIG_PLATFORM_STM_UART_NO_DEINIT
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
#endif
/*----------------------------------------------------------------------------*/
static void serialSetCallback(void *object, void (*callback)(void *),
    void *argument)
{
  struct Serial * const interface = object;

  interface->callbackArgument = argument;
  interface->callback = callback;
}
/*----------------------------------------------------------------------------*/
static enum Result serialGetParam(void *object, enum IfParameter parameter,
    void *data)
{
  struct Serial * const interface = object;

#ifdef CONFIG_PLATFORM_STM_UART_RC
  switch ((enum SerialParameter)parameter)
  {
    case IF_SERIAL_PARITY:
      *(uint8_t *)data = (uint8_t)uartGetParity(object);
      return E_OK;

    default:
      break;
  }
#endif

  switch (parameter)
  {
    case IF_AVAILABLE:
      *(size_t *)data = byteQueueSize(&interface->rxQueue);
      return E_OK;

    case IF_PENDING:
      *(size_t *)data = byteQueueSize(&interface->txQueue);
      return E_OK;

#ifdef CONFIG_PLATFORM_STM_UART_RC
    case IF_RATE:
      *(uint32_t *)data = uartGetRate(object);
      return E_OK;
#endif

    default:
      return E_INVALID;
  }
}
/*----------------------------------------------------------------------------*/
static enum Result serialSetParam(void *object, enum IfParameter parameter,
    const void *data)
{
  struct Serial * const interface = object;

#ifdef CONFIG_PLATFORM_STM_UART_RC
  switch ((enum SerialParameter)parameter)
  {
    case IF_SERIAL_PARITY:
      uartSetParity(object, (enum SerialParity)(*(const uint8_t *)data));
      return E_OK;

    default:
      break;
  }

  switch (parameter)
  {
    case IF_RATE:
      interface->rate = *(const uint32_t *)data;
      uartSetRate(object, interface->rate);
      return E_OK;

    default:
      return E_INVALID;
  }
#else
  (void)interface;
  (void)parameter;
  (void)data;

  return E_INVALID;
#endif
}
/*----------------------------------------------------------------------------*/
static size_t serialRead(void *object, void *buffer, size_t length)
{
  struct Serial * const interface = object;

  irqDisable(interface->base.irq);
  const size_t read = byteQueuePopArray(&interface->rxQueue, buffer, length);
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
