/*
 * serial.c
 * Copyright (C) 2024 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/platform/imxrt/lpuart_base.h>
#include <halm/platform/imxrt/lpuart_defs.h>
#include <halm/platform/imxrt/serial.h>
#include <halm/pm.h>
#include <xcore/containers/byte_queue.h>
#include <stdbool.h>
#include <string.h>
/*----------------------------------------------------------------------------*/
struct Serial
{
  struct LpUartBase base;

  void (*callback)(void *);
  void *callbackArgument;

  /* Input queue */
  struct ByteQueue rxQueue;
  /* Output queue */
  struct ByteQueue txQueue;
  /* Input FIFO depth */
  uint8_t rxFifoDepth;
  /* Output FIFO depth */
  uint8_t txFifoDepth;

#ifdef CONFIG_PLATFORM_IMXRT_LPUART_PM
  /* Desired baud rate */
  uint32_t rate;
#endif

#ifdef CONFIG_PLATFORM_IMXRT_LPUART_WATERMARK
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

#ifdef CONFIG_PLATFORM_IMXRT_LPUART_PM
static void powerStateHandler(void *, enum PmState);
#endif
/*----------------------------------------------------------------------------*/
static enum Result serialInit(void *, const void *);
static void serialSetCallback(void *, void (*)(void *), void *);
static enum Result serialGetParam(void *, int, void *);
static enum Result serialSetParam(void *, int, const void *);
static size_t serialRead(void *, void *, size_t);
static size_t serialWrite(void *, const void *, size_t);

#ifndef CONFIG_PLATFORM_IMXRT_LPUART_NO_DEINIT
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
  IMX_LPUART_Type * const reg = interface->base.reg;
  bool event = false;

  const uint32_t stat = reg->STAT;
  reg->STAT = stat;

  /* Handle received data */
  if (stat & STAT_RDRF)
  {
    uint32_t pending = interface->rxFifoDepth > 1 ?
        WATER_RXCOUNT_VALUE(reg->WATER) : 1;

    while (pending--)
    {
      const uint8_t data = reg->DATA;

      /* Received bytes will be dropped when queue becomes full */
      if (!byteQueueFull(&interface->rxQueue))
        byteQueuePushBack(&interface->rxQueue, data);
    }
  }

  /* Handle reception timeout */
  if (stat & STAT_IDLE)
  {
    /* Idle flag is already cleared */
    event = true;
  }

  /* Send queued data */
  if (reg->CTRL & CTRL_TIE)
  {
    uint32_t available = interface->txFifoDepth > 1 ?
        (interface->txFifoDepth - WATER_TXCOUNT_VALUE(reg->WATER)) : 1;

    updateTxWatermark(interface, byteQueueSize(&interface->txQueue));

    if (available)
    {
      if (available > byteQueueSize(&interface->txQueue))
        available = byteQueueSize(&interface->txQueue);

      while (available--)
        reg->DATA = byteQueuePopFront(&interface->txQueue);

      if (byteQueueEmpty(&interface->txQueue))
      {
        /* User function will be called when the transmit queue is empty */
        reg->CTRL &= ~CTRL_TIE;
        event = true;
      }
    }
  }

  /* User function will be called when receive queue is half-full */
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
#ifdef CONFIG_PLATFORM_IMXRT_LPUART_WATERMARK
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
#ifdef CONFIG_PLATFORM_IMXRT_LPUART_WATERMARK
  if (level > interface->txWatermark)
    interface->txWatermark = level;
#else
  (void)interface;
  (void)level;
#endif
}
/*----------------------------------------------------------------------------*/
#ifdef CONFIG_PLATFORM_IMXRT_LPUART_PM
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

  const struct LpUartBaseConfig baseConfig = {
      .rx = config->rx,
      .tx = config->tx,
      .channel = config->channel
  };
  struct Serial * const interface = object;
  enum Result res;

  /* Call base class constructor */
  if ((res = LpUartBase->init(interface, &baseConfig)) != E_OK)
    return res;

  if (!byteQueueInit(&interface->rxQueue, config->rxLength))
    return E_MEMORY;
  if (!byteQueueInit(&interface->txQueue, config->txLength))
    return E_MEMORY;

  interface->base.handler = interruptHandler;
  interface->callback = NULL;

#ifdef CONFIG_PLATFORM_IMXRT_LPUART_WATERMARK
  interface->rxWatermark = 0;
  interface->txWatermark = 0;
#endif

  IMX_LPUART_Type * const reg = interface->base.reg;
  uint32_t depth;

  /* Reset the peripheral */
  reg->GLOBAL = GLOBAL_RST;
  reg->GLOBAL = 0;

  /* Set oversampling ratio of 16, 1 stop bit, 8 data bits */
  reg->BAUD = BAUD_OSR(15);
  /* Enable RX interrupt */
  reg->CTRL = CTRL_RIE;

  /* Configure RX FIFO */
  depth = FIFO_RXFIFOSIZE_VALUE(reg->FIFO);
  interface->rxFifoDepth = depth > 0 ? (1 << (depth + 1)) : 1;

  if (interface->rxFifoDepth > 1)
  {
    depth = MIN(interface->rxFifoDepth >> 1, 3);
    reg->WATER |= WATER_RXWATER(depth);
    reg->FIFO |= FIFO_RXFE | FIFO_RXIDEN(RXIDEN_4_CHARS) | FIFO_RXFLUSH;
  }
  else
  {
    reg->CTRL |= CTRL_ILT | CTRL_IDLECFG(IDLECFG_4_CHARS) | CTRL_ILIE;
  }

  /* Configure TX FIFO */
  depth = FIFO_TXFIFOSIZE_VALUE(reg->FIFO);
  interface->txFifoDepth = depth > 0 ? (1 << (depth + 1)) : 1;

  if (interface->txFifoDepth > 1)
  {
    depth = MIN(interface->txFifoDepth >> 1, 3);
    reg->WATER |= WATER_TXWATER(depth);
    reg->FIFO |= FIFO_TXFE | FIFO_TXFLUSH;
  }

  if (!uartSetRate(&interface->base, config->rate))
    return E_VALUE;
  uartSetParity(&interface->base, config->parity);

#ifdef CONFIG_PLATFORM_IMXRT_LPUART_PM
  interface->rate = config->rate;

  if ((res = pmRegister(powerStateHandler, interface)) != E_OK)
    return res;
#endif

  /* Enable receiver and transmitter */
  reg->CTRL |= CTRL_RE | CTRL_TE;

  irqSetPriority(interface->base.irq, config->priority);
  irqEnable(interface->base.irq);

  return E_OK;
}
/*----------------------------------------------------------------------------*/
#ifndef CONFIG_PLATFORM_IMXRT_LPUART_NO_DEINIT
static void serialDeinit(void *object)
{
  struct Serial * const interface = object;
  IMX_LPUART_Type * const reg = interface->base.reg;

  irqDisable(interface->base.irq);
  reg->CTRL = 0;

#ifdef CONFIG_PLATFORM_IMXRT_LPUART_PM
  pmUnregister(interface);
#endif

  byteQueueDeinit(&interface->txQueue);
  byteQueueDeinit(&interface->rxQueue);
  LpUartBase->deinit(interface);
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

#ifdef CONFIG_PLATFORM_IMXRT_LPUART_RC
  switch ((enum SerialParameter)parameter)
  {
    case IF_SERIAL_PARITY:
      *(uint8_t *)data = (uint8_t)uartGetParity(&interface->base);
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

#ifdef CONFIG_PLATFORM_IMXRT_LPUART_WATERMARK
    case IF_RX_WATERMARK:
      *(size_t *)data = interface->rxWatermark;
      return E_OK;

    case IF_TX_WATERMARK:
      *(size_t *)data = interface->txWatermark;
      return E_OK;
#endif

#ifdef CONFIG_PLATFORM_IMXRT_LPUART_RC
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

#ifdef CONFIG_PLATFORM_IMXRT_LPUART_RC
  switch ((enum SerialParameter)parameter)
  {
    case IF_SERIAL_PARITY:
    {
      const enum SerialParity parity = *(const uint8_t *)data;

      uartSetParity(&interface->base, parity);
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
#  ifdef CONFIG_PLATFORM_IMXRT_LPUART_PM
        interface->rate = rate;
#  endif /* CONFIG_PLATFORM_IMXRT_LPUART_PM */

        return E_OK;
      }
      else
        return E_VALUE;
    }

    default:
      return E_INVALID;
  }
#else /* CONFIG_PLATFORM_IMXRT_LPUART_RC */
  (void)interface;
  (void)parameter;
  (void)data;

  return E_INVALID;
#endif /* CONFIG_PLATFORM_IMXRT_LPUART_RC */
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

  IMX_LPUART_Type * const reg = interface->base.reg;

  /* Invoke interrupt when the transmitter is idle */
  irqDisable(interface->base.irq);
  if (!byteQueueEmpty(&interface->txQueue) && !(reg->CTRL & CTRL_TIE))
    reg->CTRL |= CTRL_TIE;
  irqEnable(interface->base.irq);

  return position - (const uint8_t *)buffer;
}
