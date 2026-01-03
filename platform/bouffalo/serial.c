/*
 * serial.c
 * Copyright (C) 2026 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/platform/bouffalo/serial.h>
#include <halm/platform/bouffalo/uart_base.h>
#include <halm/platform/bouffalo/uart_defs.h>
#include <halm/pm.h>
#include <xcore/containers/byte_queue.h>
#include <stdbool.h>
#include <string.h>
/*----------------------------------------------------------------------------*/
struct Serial
{
  struct UartBase base;

  void (*callback)(void *);
  void *callbackArgument;

  /* Input queue */
  struct ByteQueue rxQueue;
  /* Output queue */
  struct ByteQueue txQueue;

#ifdef CONFIG_PLATFORM_BOUFFALO_UART_PM
  /* Desired baud rate */
  uint32_t rate;
#endif

#ifdef CONFIG_PLATFORM_BOUFFALO_UART_WATERMARK
  /* Maximum available frames in the receive queue */
  size_t rxWatermark;
  /* Maximum pending frames in the transmit queue */
  size_t txWatermark;
#endif
};
/*----------------------------------------------------------------------------*/
static void interruptHandler(void *);
static void updateRxWatermark(struct Serial *, size_t);
static void updateTxWatermark(struct Serial *, size_t);

#ifdef CONFIG_PLATFORM_BOUFFALO_UART_PM
static void powerStateHandler(void *, enum PmState);
#endif
/*----------------------------------------------------------------------------*/
static enum Result serialInit(void *, const void *);
static void serialSetCallback(void *, void (*)(void *), void *);
static enum Result serialGetParam(void *, int, void *);
static enum Result serialSetParam(void *, int, const void *);
static size_t serialRead(void *, void *, size_t);
static size_t serialWrite(void *, const void *, size_t);

#ifndef CONFIG_PLATFORM_BOUFFALO_UART_NO_DEINIT
static void serialDeinit(void *);
#else
#  define serialDeinit deletedDestructorTrap
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
  BL_UART_Type * const reg = interface->base.reg;
  bool event = false;

  const uint32_t status = reg->INT_STS;
  const uint32_t mask = reg->INT_MASK;

  /* Handle received data */
  if (status & (INT_STS_RFIN | INT_STS_RRTOIN))
  {
    const size_t count = FIFO_CONFIG1_RFICNT_VALUE(reg->FIFO_CONFIG1);

    for (size_t index = 0; index < count; ++index)
    {
      const uint8_t data = reg->FIFO_RDATA;

      /* Received bytes will be dropped when queue becomes full */
      if (!byteQueueFull(&interface->rxQueue))
        byteQueuePushBack(&interface->rxQueue, data);
    }

    /* Handle reception timeout */
    if (status & INT_STS_RRTOIN)
    {
      reg->INT_CLEAR = INT_CLEAR_RRTOCLR;
      event = true;
    }
  }

  /* Send queued data */
  if (!(mask & INT_MASK_TFMS) && (status & INT_STS_TFIN))
  {
    const size_t pending = byteQueueSize(&interface->txQueue);
    const size_t count = MIN(pending, FIFO_CONFIG1_FITH_MAX / 2);

    updateTxWatermark(interface, byteQueueSize(&interface->txQueue));

    for (size_t index = 0; index < count; ++index)
      reg->FIFO_WDATA = byteQueuePopFront(&interface->txQueue);

    if (byteQueueEmpty(&interface->txQueue))
    {
      reg->INT_MASK = mask | INT_MASK_TFMS;
      event = true;
    }
  }

  /* User handler will be called when receive queue is half-full */
  const size_t rxQueueLevel = byteQueueCapacity(&interface->rxQueue) / 2;
  const size_t rxQueueSize = byteQueueSize(&interface->rxQueue);

  updateRxWatermark(interface, rxQueueSize);
  event = event || rxQueueSize >= rxQueueLevel;

  if (event && interface->callback != NULL)
    interface->callback(interface->callbackArgument);
}
/*----------------------------------------------------------------------------*/
static void updateRxWatermark(struct Serial *interface, size_t level)
{
#ifdef CONFIG_PLATFORM_BOUFFALO_UART_WATERMARK
  if (level > interface->rxWatermark)
    interface->rxWatermark = level;
#else
  (void)interface;
  (void)level;
#endif
}
/*----------------------------------------------------------------------------*/
static void updateTxWatermark(struct Serial *interface, size_t level)
{
#ifdef CONFIG_PLATFORM_BOUFFALO_UART_WATERMARK
  if (level > interface->txWatermark)
    interface->txWatermark = level;
#else
  (void)interface;
  (void)level;
#endif
}
/*----------------------------------------------------------------------------*/
#ifdef CONFIG_PLATFORM_BOUFFALO_UART_PM
static void powerStateHandler(void *object, enum PmState state)
{
  if (state == PM_ACTIVE)
  {
    struct Serial * const interface = object;
    uartSetRate(&interface->base, interface->rate);
  }
}
#endif
/*----------------------------------------------------------------------------*/
static enum Result serialInit(void *object, const void *configBase)
{
  const struct SerialConfig * const config = configBase;
  assert(config != NULL);
  assert(config->rxLength > 0 && config->txLength > 0);

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

  if (!byteQueueInit(&interface->rxQueue, config->rxLength))
    return E_MEMORY;
  if (!byteQueueInit(&interface->txQueue, config->txLength))
    return E_MEMORY;

  interface->base.handler = interruptHandler;
  interface->callback = NULL;

#ifdef CONFIG_PLATFORM_BOUFFALO_UART_WATERMARK
  interface->rxWatermark = 0;
  interface->txWatermark = 0;
#endif

  BL_UART_Type * const reg = interface->base.reg;

  /* Disable the peripheral */
  reg->URX_CONFIG = 0;
  reg->UTX_CONFIG = 0;

  if (!uartSetRate(&interface->base, config->rate))
    return E_VALUE;
  uartSetParity(&interface->base, config->parity);

  reg->DATA_CONFIG = 0;
  reg->INT_MASK = INT_MASK_ALL & ~(INT_MASK_RFMS | INT_MASK_RRTOMASK);
  reg->INT_EN = INT_EN_TFIF | INT_EN_RFIF | INT_EN_RRTO;
  reg->URX_RTO_TIMER = URX_RTO_TIMER_RXRTOVA(32);
  reg->FIFO_CONFIG0 = FIFO_CONFIG0_TFICLR | FIFO_CONFIG0_RFICLR;
  reg->FIFO_CONFIG1 = FIFO_CONFIG1_TFITH(FIFO_CONFIG1_FITH_MAX / 2)
      | FIFO_CONFIG1_RFITH(FIFO_CONFIG1_FITH_MAX / 2);

  /* Enable receiver and transmitter */
  reg->URX_CONFIG |= UTX_CONFIG_RXBCNTD(BCNTD_8_BITS) | URX_CONFIG_EN;
  reg->UTX_CONFIG |= UTX_CONFIG_TXBCNTD(BCNTD_8_BITS) | UTX_CONFIG_EN
      | UTX_CONFIG_FRMEN;

#ifdef CONFIG_PLATFORM_BOUFFALO_UART_PM
  interface->rate = config->rate;

  if ((res = pmRegister(powerStateHandler, interface)) != E_OK)
    return res;
#endif

  irqSetPriority(interface->base.irq, config->priority);
  irqEnable(interface->base.irq);

  return E_OK;
}
/*----------------------------------------------------------------------------*/
#ifndef CONFIG_PLATFORM_BOUFFALO_UART_NO_DEINIT
static void serialDeinit(void *object)
{
  struct Serial * const interface = object;
  BL_UART_Type * const reg = interface->base.reg;

  irqDisable(interface->base.irq);
  reg->URX_CONFIG &= ~URX_CONFIG_EN;
  reg->UTX_CONFIG &= ~UTX_CONFIG_EN;

#ifdef CONFIG_PLATFORM_BOUFFALO_UART_PM
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

#ifdef CONFIG_PLATFORM_BOUFFALO_UART_RC
  switch ((enum SerialParameter)parameter)
  {
    case IF_SERIAL_PARITY:
      *(uint8_t *)data = (uint8_t)uartGetParity(&interface->base);
      return E_OK;

    case IF_SERIAL_STOPBITS:
      *(uint8_t *)data = (uint8_t)uartGetStopBits(&interface->base);
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

#ifdef CONFIG_PLATFORM_BOUFFALO_UART_WATERMARK
    case IF_RX_WATERMARK:
      *(size_t *)data = interface->rxWatermark;
      return E_OK;

    case IF_TX_WATERMARK:
      *(size_t *)data = interface->txWatermark;
      return E_OK;
#endif

#ifdef CONFIG_PLATFORM_BOUFFALO_UART_RC
    case IF_RATE:
      *(uint32_t *)data = uartGetRate(&interface->base);
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

#ifdef CONFIG_PLATFORM_BOUFFALO_UART_RC
  switch ((enum SerialParameter)parameter)
  {
    case IF_SERIAL_PARITY:
    {
      const enum SerialParity parity = *(const uint8_t *)data;

      uartSetParity(&interface->base, parity);
      return E_OK;
    }

    case IF_SERIAL_STOPBITS:
    {
      const enum SerialStopBits number = *(const uint8_t *)data;

      uartSetStopBits(&interface->base, number);
      return E_OK;
    }

    default:
      break;
  }

  switch ((enum IfParameter)parameter)
  {
    case IF_RATE:
    {
      const uint32_t rate = *(const uint32_t *)data;

      if (uartSetRate(&interface->base, rate))
      {
#  ifdef CONFIG_PLATFORM_BOUFFALO_UART_PM
        interface->rate = rate;
#  endif /* CONFIG_PLATFORM_BOUFFALO_UART_PM */

        return E_OK;
      }
      else
        return E_VALUE;
    }

    default:
      return E_INVALID;
  }
#else /* CONFIG_PLATFORM_BOUFFALO_UART_RC */
  (void)interface;
  (void)parameter;
  (void)data;

  return E_INVALID;
#endif /* CONFIG_PLATFORM_BOUFFALO_UART_RC */
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

  BL_UART_Type * const reg = interface->base.reg;

  /* Invoke interrupt when the transmitter is idle */
  irqDisable(interface->base.irq);
  if (!byteQueueEmpty(&interface->txQueue) && (reg->INT_MASK & INT_MASK_TFMS))
    reg->INT_MASK &= ~INT_MASK_TFMS;
  irqEnable(interface->base.irq);

  return position - (const uint8_t *)buffer;
}
