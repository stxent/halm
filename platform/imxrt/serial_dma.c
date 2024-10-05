/*
 * serial_dma.c
 * Copyright (C) 2024 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/core/cortex/cache.h>
#include <halm/platform/imxrt/edma_circular.h>
#include <halm/platform/imxrt/edma_oneshot.h>
#include <halm/platform/imxrt/lpuart_base.h>
#include <halm/platform/imxrt/lpuart_defs.h>
#include <halm/platform/imxrt/serial_dma.h>
#include <halm/pm.h>
#include <xcore/containers/byte_queue.h>
#include <malloc.h>
#include <stdbool.h>
#include <string.h>
/*----------------------------------------------------------------------------*/
#define MEM_ALIGNMENT CONFIG_CORE_CORTEX_CACHE_LINE
/*----------------------------------------------------------------------------*/
struct SerialDma
{
  struct LpUartBase base;

  void (*callback)(void *);
  void *callbackArgument;

  /* DMA channel for data reception */
  struct Dma *rxDma;
  /* DMA channel for data transmission */
  struct Dma *txDma;

  /* Input queue */
  struct ByteQueue rxQueue;
  /* Output queue */
  struct ByteQueue txQueue;
  /* Pointer to the temporary reception buffer */
  uint8_t *rxBuffer;
  /* Position inside the temporary buffer */
  size_t rxPosition;

  /* Size of the circular reception buffer */
  size_t rxBufferSize;
  /* Size of the DMA TX transfer */
  size_t txBufferSize;

  /* Queues are allocated in a static memory */
  bool preallocated;

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
static bool dmaSetup(struct SerialDma *, uint8_t, uint8_t, enum EdmaPriority);
static enum Result enqueueRxBuffer(struct SerialDma *);
static enum Result enqueueTxBuffers(struct SerialDma *);
static bool readResidue(struct SerialDma *);
static void rxDmaHandler(void *);
static void serialInterruptHandler(void *);
static void txDmaHandler(void *);
static void updateRxWatermark(struct SerialDma *, size_t);
static void updateTxWatermark(struct SerialDma *, size_t);

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
const struct InterfaceClass * const SerialDma = &(const struct InterfaceClass){
    .size = sizeof(struct SerialDma),
    .init = serialInit,
    .deinit = serialDeinit,

    .setCallback = serialSetCallback,
    .getParam = serialGetParam,
    .setParam = serialSetParam,
    .read = serialRead,
    .write = serialWrite
};
/*----------------------------------------------------------------------------*/
static bool dmaSetup(struct SerialDma *interface, uint8_t rxChannel,
    uint8_t txChannel, enum EdmaPriority priority)
{
  static const struct EdmaSettings dmaSettings[] = {
      {
          .burst = 1,
          .source = {
              .offset = 0,
              .width = DMA_WIDTH_BYTE
          },
          .destination = {
              .offset = 1,
              .width = DMA_WIDTH_BYTE
          }
      }, {
          .burst = 1,
          .source = {
              .offset = 1,
              .width = DMA_WIDTH_BYTE
          },
          .destination = {
              .offset = 0,
              .width = DMA_WIDTH_BYTE
          }
      }
  };

  const struct EdmaCircularConfig rxDmaConfig = {
      .event = edmaGetEventLpUartRx(interface->base.channel),
      .priority = priority,
      .channel = rxChannel,
      .silent = false
  };
  const struct EdmaOneShotConfig txDmaConfig = {
      .event = edmaGetEventLpUartTx(interface->base.channel),
      .priority = priority,
      .channel = txChannel
  };

  interface->rxDma = init(EdmaCircular, &rxDmaConfig);
  if (interface->rxDma == NULL)
    return false;
  dmaConfigure(interface->rxDma, &dmaSettings[0]);
  dmaSetCallback(interface->rxDma, rxDmaHandler, interface);

  interface->txDma = init(EdmaOneShot, &txDmaConfig);
  if (interface->txDma == NULL)
    return false;
  dmaConfigure(interface->txDma, &dmaSettings[1]);
  dmaSetCallback(interface->txDma, txDmaHandler, interface);

  return true;
}
/*----------------------------------------------------------------------------*/
static enum Result enqueueRxBuffer(struct SerialDma *interface)
{
  const IMX_LPUART_Type * const reg = interface->base.reg;

  dmaAppend(interface->rxDma, interface->rxBuffer, (const void *)&reg->DATA,
      interface->rxBufferSize);
  interface->rxPosition = 0;

  /* Start reception */
  return dmaEnable(interface->rxDma);
}
/*----------------------------------------------------------------------------*/
static enum Result enqueueTxBuffers(struct SerialDma *interface)
{
  IMX_LPUART_Type * const reg = interface->base.reg;
  const uint8_t *address;
  enum Result res;

  byteQueueDeferredPop(&interface->txQueue, &address,
      &interface->txBufferSize, 0);
  dmaAppend(interface->txDma, (void *)&reg->DATA, address,
      interface->txBufferSize);

  if ((res = dmaEnable(interface->txDma)) != E_OK)
    interface->txBufferSize = 0;

  return res;
}
/*----------------------------------------------------------------------------*/
static bool readResidue(struct SerialDma *interface)
{
  size_t residue;

  if (dmaResidue(interface->rxDma, &residue) != E_OK)
    return false;

  const size_t pending = interface->rxBufferSize - interface->rxPosition;

  if (residue > pending)
  {
    /*
     * DMA transfer is completed but DMA interrupt is not yet processed.
     * Ignore this event and process received bytes later in the DMA callback.
     */
    return false;
  }

  const size_t count = pending - residue;

  if (count)
  {
    const uint8_t * const address = interface->rxBuffer + interface->rxPosition;

    dCacheInvalidate((uintptr_t)address, count);
    byteQueuePushArray(&interface->rxQueue, address, count);
    interface->rxPosition += count;

    updateRxWatermark(interface, byteQueueSize(&interface->rxQueue));
  }

  return true;
}
/*----------------------------------------------------------------------------*/
static void rxDmaHandler(void *object)
{
  struct SerialDma * const interface = object;
  const size_t index = dmaQueued(interface->rxDma);
  bool event = false;

  assert(index >= 1 && index <= 2);
  assert(dmaStatus(interface->rxDma) == E_BUSY);

  const size_t end = interface->rxBufferSize >> (2 - index);
  const size_t count = end - interface->rxPosition;

  if (count)
  {
    const uint8_t * const address = interface->rxBuffer + interface->rxPosition;

    dCacheInvalidate((uintptr_t)address, count);
    byteQueuePushArray(&interface->rxQueue, address, count);
    updateRxWatermark(interface, byteQueueSize(&interface->rxQueue));

    event = true;
  }

  interface->rxPosition = index == 1 ? (interface->rxBufferSize >> 1) : 0;

  if (event && interface->callback != NULL)
    interface->callback(interface->callbackArgument);
}
/*----------------------------------------------------------------------------*/
static void serialInterruptHandler(void *object)
{
  struct SerialDma * const interface = object;
  IMX_LPUART_Type * const reg = interface->base.reg;

  const uint32_t stat = reg->STAT;
  reg->STAT = stat;

  /* Handle reception timeout */
  const bool event = readResidue(interface);

  if (event && interface->callback != NULL)
    interface->callback(interface->callbackArgument);
}
/*----------------------------------------------------------------------------*/
static void txDmaHandler(void *object)
{
  struct SerialDma * const interface = object;
  bool event = false;

  if (dmaStatus(interface->txDma) != E_ERROR)
    byteQueueAbandon(&interface->txQueue, interface->txBufferSize);
  interface->txBufferSize = 0;

  if (!byteQueueEmpty(&interface->txQueue))
    enqueueTxBuffers(interface);
  else
    event = true;

  if (event && interface->callback != NULL)
    interface->callback(interface->callbackArgument);
}
/*----------------------------------------------------------------------------*/
static void updateRxWatermark(struct SerialDma *interface, size_t level)
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
static void updateTxWatermark(struct SerialDma *interface, size_t level)
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
    struct SerialDma * const interface = object;
    uartSetRate(&interface->base, interface->rate);
  }
}
#endif
/*----------------------------------------------------------------------------*/
static enum Result serialInit(void *object, const void *configBase)
{
  const struct SerialDmaConfig * const config = configBase;
  assert(config != NULL);
  assert(config->rxChunk > 0 && config->rxLength > 0 && config->txLength > 0);
  assert(config->rxChunk % MEM_ALIGNMENT == 0);
  assert(config->txLength % MEM_ALIGNMENT == 0);
  assert(config->rxLength % config->rxChunk == 0);

  const struct LpUartBaseConfig baseConfig = {
      .rx = config->rx,
      .tx = config->tx,
      .channel = config->channel
  };
  struct SerialDma * const interface = object;
  enum Result res;

  /* Call base class constructor */
  if ((res = LpUartBase->init(interface, &baseConfig)) != E_OK)
    return res;

  if (config->arena != NULL)
  {
    uint8_t * const arena = config->arena;

    byteQueueInitArena(&interface->txQueue, config->txLength, arena);
    byteQueueInitArena(&interface->rxQueue, config->rxLength,
        arena + config->txLength);

    interface->preallocated = true;
  }
  else
  {
    /* Output queue buffer is used directly by DMA and should be aligned */
    if (!byteQueueInitAligned(&interface->txQueue, config->txLength,
        MEM_ALIGNMENT))
    {
      return E_MEMORY;
    }

    if (!byteQueueInit(&interface->rxQueue, config->rxLength))
      return E_MEMORY;

    interface->preallocated = false;
  }

  /* Allocate aligned memory chunk for a circular input buffer */
  interface->rxBuffer = memalign(MEM_ALIGNMENT, config->rxChunk);
  if (interface->rxBuffer == NULL)
    return E_MEMORY;

  interface->base.handler = serialInterruptHandler;
  interface->callback = NULL;
  interface->rxBufferSize = config->rxChunk;
  interface->txBufferSize = 0;

#ifdef CONFIG_PLATFORM_IMXRT_LPUART_WATERMARK
  interface->rxWatermark = 0;
  interface->txWatermark = 0;
#endif

  if (!dmaSetup(interface, config->dma[0], config->dma[1], config->priority))
    return E_ERROR;

  IMX_LPUART_Type * const reg = interface->base.reg;

  /* Reset the peripheral */
  reg->GLOBAL = GLOBAL_RST;
  reg->GLOBAL = 0;

  /* Set oversampling ratio of 16, 1 stop bit, 8 data bits, enable DMA */
  reg->BAUD = BAUD_RDMAE | BAUD_TDMAE | BAUD_OSR(15);
  /* Enable RX IDLE interrupt */
  reg->CTRL = CTRL_ILT | CTRL_IDLECFG(IDLECFG_4_CHARS) | CTRL_ILIE;

  /* Configure RX FIFO */
  if (FIFO_RXFIFOSIZE_VALUE(reg->FIFO) > 0)
  {
    reg->WATER |= WATER_RXWATER(1);
    reg->FIFO |= FIFO_RXFE | FIFO_RXIDEN(RXIDEN_2_CHARS) | FIFO_RXFLUSH;
  }

  /* Configure TX FIFO */
  const uint32_t depth = FIFO_TXFIFOSIZE_VALUE(reg->FIFO);

  if (depth > 0)
  {
    reg->WATER |= WATER_TXWATER(MIN((1 << depth), 3));
    reg->FIFO |= FIFO_TXFE | FIFO_TXFLUSH;
  }

  /* Configure baud rate */
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

  irqSetPriority(interface->base.irq, CONFIG_PLATFORM_IMXRT_EDMA_PRIORITY);
  irqEnable(interface->base.irq);

  return enqueueRxBuffer(interface);
}
/*----------------------------------------------------------------------------*/
#ifndef CONFIG_PLATFORM_IMXRT_LPUART_NO_DEINIT
static void serialDeinit(void *object)
{
  struct SerialDma * const interface = object;
  IMX_LPUART_Type * const reg = interface->base.reg;

  /* Stop DMA channels */
  dmaDisable(interface->txDma);
  dmaDisable(interface->rxDma);

  irqDisable(interface->base.irq);
  reg->CTRL = 0;

#ifdef CONFIG_PLATFORM_IMXRT_LPUART_PM
  pmUnregister(interface);
#endif

  /* Free DMA channels */
  deinit(interface->txDma);
  deinit(interface->rxDma);

  /* Free the reception buffer */
  free(interface->rxBuffer);

  if (interface->preallocated)
  {
    byteQueueDeinitArena(&interface->rxQueue);
    byteQueueDeinitArena(&interface->txQueue);
  }
  else
  {
    /* Free memory allocated for queues */
    byteQueueDeinit(&interface->rxQueue);
    byteQueueDeinit(&interface->txQueue);
  }

  LpUartBase->deinit(interface);
}
#endif
/*----------------------------------------------------------------------------*/
static void serialSetCallback(void *object, void (*callback)(void *),
    void *argument)
{
  struct SerialDma * const interface = object;

  interface->callbackArgument = argument;
  interface->callback = callback;
}
/*----------------------------------------------------------------------------*/
static enum Result serialGetParam(void *object, int parameter, void *data)
{
  struct SerialDma * const interface = object;

#ifdef CONFIG_PLATFORM_IMXRT_LPUART_RC
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
  struct SerialDma * const interface = object;

#ifdef CONFIG_PLATFORM_IMXRT_LPUART_RC
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
  struct SerialDma * const interface = object;
  uint8_t *position = buffer;

  while (length && !byteQueueEmpty(&interface->rxQueue))
  {
    const uint8_t *address;
    size_t count;

    byteQueueDeferredPop(&interface->rxQueue, &address, &count, 0);
    count = MIN(length, count);
    memcpy(position, address, count);

    const IrqState state = irqSave();
    byteQueueAbandon(&interface->rxQueue, count);
    irqRestore(state);

    position += count;
    length -= count;
  }

  return position - (uint8_t *)buffer;
}
/*----------------------------------------------------------------------------*/
static size_t serialWrite(void *object, const void *buffer, size_t length)
{
  struct SerialDma * const interface = object;
  const uint8_t *position = buffer;
  IrqState state;

  while (length && !byteQueueFull(&interface->txQueue))
  {
    uint8_t *address;
    size_t count;

    byteQueueDeferredPush(&interface->txQueue, &address, &count, 0);
    count = MIN(length, count);
    memcpy(address, position, count);

    state = irqSave();
    byteQueueAdvance(&interface->txQueue, count);
    dCacheClean((uintptr_t)address, count);
    irqRestore(state);

    position += count;
    length -= count;
  }

  state = irqSave();
  updateTxWatermark(interface, byteQueueSize(&interface->txQueue));
  if (!byteQueueEmpty(&interface->txQueue) && interface->txBufferSize == 0)
    enqueueTxBuffers(interface);
  irqRestore(state);

  return position - (const uint8_t *)buffer;
}
