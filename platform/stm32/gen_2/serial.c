/*
 * serial.c
 * Copyright (C) 2020 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/generic/byte_queue_extensions.h>
#include <halm/platform/stm32/serial.h>
#include <halm/platform/stm32/uart_defs.h>
#include <stdbool.h>
#include <string.h>
/*----------------------------------------------------------------------------*/
static void interruptHandler(void *);
/*----------------------------------------------------------------------------*/
static enum Result serialInit(void *, const void *);
static void serialSetCallback(void *, void (*)(void *), void *);
static enum Result serialGetParam(void *, int, void *);
static enum Result serialSetParam(void *, int, const void *);
static size_t serialRead(void *, void *, size_t);
static size_t serialWrite(void *, const void *, size_t);

#ifndef CONFIG_PLATFORM_STM32_UART_NO_DEINIT
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

  const uint32_t status = reg->ISR;

  /* Handle received data */
  if (status & ISR_RXNE)
  {
    const uint8_t data = reg->RDR;

    /* Received bytes will be dropped when queue becomes full */
    if (!byteQueueFull(&interface->rxQueue))
      byteQueuePushBack(&interface->rxQueue, data);
  }

  /* Handle reception timeout */
  if (status & ISR_IDLE)
  {
    /* Clear IDLE flag */
    reg->ICR = ICR_IDLECF;
    event = true;
  }

  const uint32_t control = reg->CR1;

  /* Send queued data */
  if ((control & CR1_TXEIE) && (status & ISR_TXE))
  {
    if (!byteQueueEmpty(&interface->txQueue))
      reg->TDR = byteQueuePopFront(&interface->txQueue);
    else
      reg->ICR = ICR_TCCF;

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
  if ((res = UartBase->init(interface, &baseConfig)) != E_OK)
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

  /* Disable overrun error */
  reg->CR3 = CR3_OVRDIS;
  /* Clear pending interrupt flags */
  reg->ICR = ICR_IDLECF | ICR_TCCF;
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
#ifndef CONFIG_PLATFORM_STM32_UART_NO_DEINIT
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
static enum Result serialGetParam(void *object, int parameter, void *data)
{
  struct Serial * const interface = object;

#ifdef CONFIG_PLATFORM_STM32_UART_RC
  switch ((enum SerialParameter)parameter)
  {
    case IF_SERIAL_PARITY:
      *(uint8_t *)data = (uint8_t)uartGetParity(object);
      return E_OK;

    default:
      break;
  }
#endif

  switch ((enum IfParameter)parameter)
  {
    case IF_RX_AVAILABLE:
      *(size_t *)data = byteQueueSize(&interface->rxQueue);
      return E_OK;

    case IF_RX_PENDING:
      *(size_t *)data = byteQueueCapacity(&interface->rxQueue)
          - byteQueueSize(&interface->rxQueue);
      return E_OK;

    case IF_TX_AVAILABLE:
      *(size_t *)data = byteQueueCapacity(&interface->txQueue)
          - byteQueueSize(&interface->txQueue);
      return E_OK;

    case IF_TX_PENDING:
      *(size_t *)data = byteQueueSize(&interface->txQueue);
      return E_OK;

#ifdef CONFIG_PLATFORM_STM32_UART_RC
    case IF_RATE:
      *(uint32_t *)data = uartGetRate(object);
      return E_OK;
#endif

    default:
      return E_INVALID;
  }
}
/*----------------------------------------------------------------------------*/
static enum Result serialSetParam(void *object, int parameter, const void *data)
{
  struct Serial * const interface = object;

#ifdef CONFIG_PLATFORM_STM32_UART_RC
  switch ((enum SerialParameter)parameter)
  {
    case IF_SERIAL_PARITY:
      uartSetParity(object, (enum SerialParity)(*(const uint8_t *)data));
      return E_OK;

    default:
      break;
  }

  switch ((enum IfParameter)parameter)
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
  uint8_t *position = buffer;

  while (length && !byteQueueEmpty(&interface->rxQueue))
  {
    const uint8_t *address;
    size_t count;

    byteQueueDeferredPop(&interface->rxQueue, &address, &count, 0);
    count = MIN(length, count);
    memcpy(position, address, count);

    irqDisable(interface->base.irq);
    byteQueueAbandon(&interface->rxQueue, count);
    irqEnable(interface->base.irq);

    position += count;
    length -= count;
  }

  return position - (uint8_t *)buffer;
}
/*----------------------------------------------------------------------------*/
static size_t serialWrite(void *object, const void *buffer, size_t length)
{
  struct Serial * const interface = object;
  const uint8_t *position = buffer;

  while (length && !byteQueueFull(&interface->txQueue))
  {
    uint8_t *address;
    size_t count;

    byteQueueDeferredPush(&interface->txQueue, &address, &count, 0);
    count = MIN(length, count);
    memcpy(address, position, count);

    irqDisable(interface->base.irq);
    byteQueueAdvance(&interface->txQueue, count);
    irqEnable(interface->base.irq);

    position += count;
    length -= count;
  }

  STM_USART_Type * const reg = interface->base.reg;

  irqDisable(interface->base.irq);
  if (!byteQueueEmpty(&interface->txQueue) && !(reg->CR1 & CR1_TXEIE))
    reg->CR1 |= CR1_TXEIE;
  irqEnable(interface->base.irq);

  return position - (const uint8_t *)buffer;
}
