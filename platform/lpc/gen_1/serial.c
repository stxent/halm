/*
 * serial.c
 * Copyright (C) 2012 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/generic/byte_queue_extensions.h>
#include <halm/platform/lpc/gen_1/uart_defs.h>
#include <halm/platform/lpc/serial.h>
#include <halm/pm.h>
#include <stdbool.h>
#include <string.h>
/*----------------------------------------------------------------------------*/
#define TX_FIFO_SIZE 8
/*----------------------------------------------------------------------------*/
static void interruptHandler(void *);

#ifdef CONFIG_PLATFORM_LPC_UART_PM
static void powerStateHandler(void *, enum PmState);
#endif
/*----------------------------------------------------------------------------*/
static enum Result serialInit(void *, const void *);
static void serialSetCallback(void *, void (*)(void *), void *);
static enum Result serialGetParam(void *, int, void *);
static enum Result serialSetParam(void *, int, const void *);
static size_t serialRead(void *, void *, size_t);
static size_t serialWrite(void *, const void *, size_t);

#ifndef CONFIG_PLATFORM_LPC_UART_NO_DEINIT
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
  LPC_UART_Type * const reg = interface->base.reg;
  bool event;

  /* Interrupt status cleared when performed read operation on IIR register */
  const uint32_t state = reg->IIR;

  /* Call user handler when receive timeout occurs */
  event = (state & IIR_INTID_MASK) == IIR_INTID_CTI;

  /* Byte will be removed from FIFO after reading from RBR register */
  while (reg->LSR & LSR_RDR)
  {
    const uint8_t data = reg->RBR;

    /* Received bytes will be dropped when queue becomes full */
    if (!byteQueueFull(&interface->rxQueue))
      byteQueuePushBack(&interface->rxQueue, data);
  }
  if (reg->LSR & LSR_THRE)
  {
    const size_t txQueueSize = byteQueueSize(&interface->txQueue);

    if (txQueueSize > 0)
    {
      /* Fill FIFO with selected burst size or less */
      size_t bytesToWrite = MIN(txQueueSize, TX_FIFO_SIZE);

      /* Call user handler when transmit queue is empty */
      event = event || bytesToWrite == txQueueSize;

      while (bytesToWrite--)
        reg->THR = byteQueuePopFront(&interface->txQueue);
    }
  }

  /* User handler will be called when receive queue is half-full */
  const size_t rxQueueLevel = byteQueueCapacity(&interface->rxQueue) / 2;
  const size_t rxQueueSize = byteQueueSize(&interface->rxQueue);

  event = event || rxQueueSize >= rxQueueLevel;

  if (interface->callback && event)
    interface->callback(interface->callbackArgument);
}
/*----------------------------------------------------------------------------*/
#ifdef CONFIG_PLATFORM_LPC_UART_PM
static void powerStateHandler(void *object, enum PmState state)
{
  struct Serial * const interface = object;

  if (state == PM_ACTIVE)
  {
    struct UartRateConfig rateConfig;

    /* Recalculate and set baud rate */
    if (uartCalcRate(object, interface->rate, &rateConfig) == E_OK)
      uartSetRate(object, rateConfig);
  }
}
#endif
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
  struct UartRateConfig rateConfig;
  enum Result res;

  /* Call base class constructor */
  if ((res = UartBase->init(interface, &baseConfig)) != E_OK)
    return res;

  if ((res = uartCalcRate(object, config->rate, &rateConfig)) != E_OK)
    return res;

  interface->base.handler = interruptHandler;

  interface->callback = 0;
  interface->rate = config->rate;

  if (!byteQueueInit(&interface->rxQueue, config->rxLength))
    return E_MEMORY;
  if (!byteQueueInit(&interface->txQueue, config->txLength))
    return E_MEMORY;

  LPC_UART_Type * const reg = interface->base.reg;

  /* Set 8-bit length */
  reg->LCR = LCR_WLS(WLS_8BIT);
  /* Enable FIFO and set RX trigger level */
  reg->FCR = (reg->FCR & ~FCR_RXTRIGLVL_MASK) | FCR_FIFOEN
      | FCR_RXTRIGLVL(RX_TRIGGER_LEVEL_8);
  /* Enable RBR and THRE interrupts */
  reg->IER = IER_RBRINTEN | IER_THREINTEN;
  /* Transmitter is enabled by default thus TER register is left untouched */

  uartSetParity(object, config->parity);
  uartSetRate(object, rateConfig);

#ifdef CONFIG_PLATFORM_LPC_UART_PM
  if ((res = pmRegister(powerStateHandler, interface)) != E_OK)
    return res;
#endif

  irqSetPriority(interface->base.irq, config->priority);
  irqEnable(interface->base.irq);

  return E_OK;
}
/*----------------------------------------------------------------------------*/
#ifndef CONFIG_PLATFORM_LPC_UART_NO_DEINIT
static void serialDeinit(void *object)
{
  struct Serial * const interface = object;

  irqDisable(interface->base.irq);

#ifdef CONFIG_PLATFORM_LPC_UART_PM
  pmUnregister(interface);
#endif

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

#ifdef CONFIG_PLATFORM_LPC_UART_RC
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
    case IF_AVAILABLE:
      *(size_t *)data = byteQueueSize(&interface->rxQueue);
      return E_OK;

    case IF_PENDING:
      *(size_t *)data = byteQueueSize(&interface->txQueue);
      return E_OK;

#ifdef CONFIG_PLATFORM_LPC_UART_RC
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

#ifdef CONFIG_PLATFORM_LPC_UART_RC
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
    {
      struct UartRateConfig rateConfig;
      const enum Result res = uartCalcRate(object, *(const uint32_t *)data,
          &rateConfig);

      if (res == E_OK)
      {
        interface->rate = *(const uint32_t *)data;
        uartSetRate(object, rateConfig);
      }
      return res;
    }

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

  LPC_UART_Type * const reg = interface->base.reg;

  /* Invoke interrupt when the transmitter is idle */
  irqDisable(interface->base.irq);
  if (!byteQueueEmpty(&interface->txQueue) && (reg->LSR & LSR_THRE))
    irqSetPending(interface->base.irq);
  irqEnable(interface->base.irq);

  return position - (const uint8_t *)buffer;
}
