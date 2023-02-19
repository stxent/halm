/*
 * serial_dma.c
 * Copyright (C) 2023 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/generic/byte_queue_extensions.h>
#include <halm/platform/numicro/pdma_list.h>
#include <halm/platform/numicro/pdma_oneshot.h>
#include <halm/platform/numicro/serial_dma.h>
#include <halm/platform/numicro/uart_base.h>
#include <halm/platform/numicro/uart_defs.h>
#include <halm/pm.h>
#include <stdlib.h>
#include <string.h>
/*----------------------------------------------------------------------------*/
struct SerialDma
{
  struct UartBase base;

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
  /* Queues are allocated in a static memory */
  bool preallocated;

  /* Count of reception buffers */
  size_t rxChunks;
  /* Position inside the current receive buffer */
  size_t rxPosition;
  /* Size of the single reception buffer */
  size_t rxBufferSize;
  /* Size of the current outgoing transfer */
  size_t txBufferSize;

#ifdef CONFIG_PLATFORM_NUMICRO_UART_PM
  /* Desired baud rate */
  uint32_t rate;
#endif

#ifdef CONFIG_PLATFORM_NUMICRO_UART_WATERMARK
  /* Maximum available frames in the receive queue */
  size_t rxWatermark;
  /* Maximum pending frames in the transmit queue */
  size_t txWatermark;
#endif
};
/*----------------------------------------------------------------------------*/
static bool dmaSetup(struct SerialDma *, uint8_t, uint8_t, size_t);
static enum Result enqueueRxBuffers(struct SerialDma *);
static enum Result enqueueTxBuffers(struct SerialDma *);
static void readResidue(struct SerialDma *);
static void rxDmaHandler(void *);
static bool rxQueueReady(struct SerialDma *);
static void txDmaHandler(void *);
static void updateRxWatermark(struct SerialDma *, size_t);
static void updateTxWatermark(struct SerialDma *, size_t);

#ifdef CONFIG_PLATFORM_NUMICRO_UART_PM
static void powerStateHandler(void *, enum PmState);
#endif
/*----------------------------------------------------------------------------*/
static enum Result serialInit(void *, const void *);
static void serialSetCallback(void *, void (*)(void *), void *);
static enum Result serialGetParam(void *, int, void *);
static enum Result serialSetParam(void *, int, const void *);
static size_t serialRead(void *, void *, size_t);
static size_t serialWrite(void *, const void *, size_t);

#ifndef CONFIG_PLATFORM_NUMICRO_UART_NO_DEINIT
static void serialDeinit(void *);
#else
#define serialDeinit deletedDestructorTrap
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
    uint8_t txChannel, size_t chunks)
{
  static const struct PdmaSettings dmaSettings[] = {
      {
          .burst = DMA_BURST_1,
          .priority = DMA_PRIORITY_FIXED,
          .width = DMA_WIDTH_BYTE,
          .source = {
              .increment = false
          },
          .destination = {
              .increment = true
          }
      }, {
          .burst = DMA_BURST_1,
          .priority = DMA_PRIORITY_FIXED,
          .width = DMA_WIDTH_BYTE,
          .source = {
              .increment = true
          },
          .destination = {
              .increment = false
          }
      }
  };
  const struct PdmaListConfig rxDmaConfig = {
      .number = chunks,
      .event = pdmaGetEventUartRx(interface->base.channel),
      .channel = rxChannel
  };
  const struct PdmaOneShotConfig txDmaConfig = {
      .event = pdmaGetEventUartTx(interface->base.channel),
      .channel = txChannel
  };

  interface->rxDma = init(PdmaList, &rxDmaConfig);
  if (!interface->rxDma)
    return false;
  dmaConfigure(interface->rxDma, &dmaSettings[0]);
  dmaSetCallback(interface->rxDma, rxDmaHandler, interface);

  interface->txDma = init(PdmaOneShot, &txDmaConfig);
  if (!interface->txDma)
    return false;
  dmaConfigure(interface->txDma, &dmaSettings[1]);
  dmaSetCallback(interface->txDma, txDmaHandler, interface);

  return true;
}
/*----------------------------------------------------------------------------*/
static enum Result enqueueRxBuffers(struct SerialDma *interface)
{
  const size_t pending = dmaQueued(interface->rxDma);
  uint8_t *address;
  size_t count;

  byteQueueDeferredPush(&interface->rxQueue, &address, &count,
      pending * interface->rxBufferSize - interface->rxPosition);

  if (count >= interface->rxBufferSize)
  {
    NM_UART_Type * const reg = interface->base.reg;

    do
    {
      dmaAppend(interface->rxDma, address, (void *)&reg->DAT,
          interface->rxBufferSize);

      address += interface->rxBufferSize;
      count -= interface->rxBufferSize;
    }
    while (count >= interface->rxBufferSize);

    if (dmaStatus(interface->rxDma) != E_BUSY)
      return dmaEnable(interface->rxDma);
  }

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static enum Result enqueueTxBuffers(struct SerialDma *interface)
{
  NM_UART_Type * const reg = interface->base.reg;
  const uint8_t *address;
  enum Result res;

  updateTxWatermark(interface, byteQueueSize(&interface->txQueue));

  byteQueueDeferredPop(&interface->txQueue, &address,
      &interface->txBufferSize, 0);
  dmaAppend(interface->txDma, (void *)&reg->DAT, address,
      interface->txBufferSize);

  if ((res = dmaEnable(interface->txDma)) != E_OK)
    interface->txBufferSize = 0;

  return res;
}
/*----------------------------------------------------------------------------*/
static void readResidue(struct SerialDma *interface)
{
  size_t residue;

  if (dmaResidue(interface->rxDma, &residue) == E_OK)
  {
    const size_t pending =
        interface->rxBufferSize - interface->rxPosition - residue;

    if (pending)
    {
      byteQueueAdvance(&interface->rxQueue, pending);
      interface->rxPosition += pending;

      updateRxWatermark(interface, byteQueueSize(&interface->rxQueue));
    }
  }
}
/*----------------------------------------------------------------------------*/
static void rxDmaHandler(void *object)
{
  struct SerialDma * const interface = object;
  const bool event = dmaStatus(interface->rxDma) != E_ERROR;

  if (event)
  {
    const size_t pending = interface->rxBufferSize - interface->rxPosition;

    byteQueueAdvance(&interface->rxQueue, pending);
    interface->rxPosition = 0;

    updateRxWatermark(interface, byteQueueSize(&interface->rxQueue));
  }

  if (rxQueueReady(interface))
    enqueueRxBuffers(interface);

  if (event && interface->callback)
    interface->callback(interface->callbackArgument);
}
/*----------------------------------------------------------------------------*/
static bool rxQueueReady(struct SerialDma *interface)
{
  const size_t pending = dmaQueued(interface->rxDma);

  if (pending != 1)
  {
    const size_t available = byteQueueCapacity(&interface->rxQueue)
        - byteQueueSize(&interface->rxQueue)
        - pending * interface->rxBufferSize;

    if (available >= interface->rxBufferSize)
      return true;
  }

  return false;
}
/*----------------------------------------------------------------------------*/
static void txDmaHandler(void *object)
{
  struct SerialDma * const interface = object;
  bool event = false;

  if (dmaStatus(interface->rxDma) != E_ERROR)
    byteQueueAbandon(&interface->txQueue, interface->txBufferSize);
  interface->txBufferSize = 0;

  if (!byteQueueEmpty(&interface->txQueue))
    enqueueTxBuffers(interface);
  else
    event = true;

  if (event && interface->callback)
    interface->callback(interface->callbackArgument);
}
/*----------------------------------------------------------------------------*/
static void updateRxWatermark(struct SerialDma *interface, size_t level)
{
#ifdef CONFIG_PLATFORM_NUMICRO_UART_WATERMARK
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
#ifdef CONFIG_PLATFORM_NUMICRO_UART_WATERMARK
  if (level > interface->txWatermark)
    interface->txWatermark = level;
#else
  (void)interface;
  (void)level;
#endif
}
/*----------------------------------------------------------------------------*/
#ifdef CONFIG_PLATFORM_NUMICRO_UART_PM
static void powerStateHandler(void *object, enum PmState state)
{
  struct SerialDma * const interface = object;

  if (state == PM_ACTIVE)
    uartSetRate(&interface->base, interface->rate);
}
#endif
/*----------------------------------------------------------------------------*/
static enum Result serialInit(void *object, const void *configBase)
{
  const struct SerialDmaConfig * const config = configBase;
  assert(config);
  assert(config->rxLength % config->rxChunks == 0);

  const struct UartBaseConfig baseConfig = {
      .rx = config->rx,
      .tx = config->tx,
      .priority = config->priority,
      .channel = config->channel
  };
  struct SerialDma * const interface = object;
  enum Result res;

  /* Call base class constructor */
  if ((res = UartBase->init(interface, &baseConfig)) != E_OK)
    return res;

  if (config->arena)
  {
    uint8_t * const arena = config->arena;

    byteQueueInitArena(&interface->rxQueue, config->rxLength, arena);
    byteQueueInitArena(&interface->txQueue, config->txLength,
        arena + config->rxLength);

    interface->preallocated = true;
  }
  else
  {
    if (!byteQueueInit(&interface->rxQueue, config->rxLength))
      return E_MEMORY;
    if (!byteQueueInit(&interface->txQueue, config->txLength))
      return E_MEMORY;

    interface->preallocated = false;
  }

  if (!dmaSetup(interface, config->dma[0], config->dma[1], config->rxChunks))
    return E_ERROR;

  interface->callback = 0;
  interface->rxChunks = config->rxChunks;
  interface->rxPosition = 0;
  interface->rxBufferSize = config->rxLength / config->rxChunks;
  interface->txBufferSize = 0;

#ifdef CONFIG_PLATFORM_NUMICRO_UART_WATERMARK
  interface->rxWatermark = 0;
  interface->txWatermark = 0;
#endif

  NM_UART_Type * const reg = interface->base.reg;

  /* Disable interrupts, disable receiver and transmitter */
  reg->INTEN = 0;
  reg->FUNCSEL = FUNCSEL_TXRXDIS;
  while (reg->FIFOSTS & FIFOSTS_TXRXACT);

  reg->LINE = LINE_WLS(WLS_8BIT);
  reg->FIFO = FIFO_RXRST | FIFO_TXRST;
  reg->TOUT = 0;

  uartSetRate(object, config->rate);
  uartSetParity(object, config->parity);

  reg->FUNCSEL = FUNCSEL_FUNCSEL(FUNCSEL_UART);
  reg->INTEN = INTEN_TXPDMAEN | INTEN_RXPDMAEN;

#ifdef CONFIG_PLATFORM_NUMICRO_UART_PM
  interface->rate = config->rate;

  if ((res = pmRegister(powerStateHandler, interface)) != E_OK)
    return res;
#endif

  return enqueueRxBuffers(interface);
}
/*----------------------------------------------------------------------------*/
#ifndef CONFIG_PLATFORM_NUMICRO_UART_NO_DEINIT
static void serialDeinit(void *object)
{
  struct SerialDma * const interface = object;
  NM_UART_Type * const reg = interface->base.reg;

  /* Stop DMA channels */
  dmaDisable(interface->txDma);
  dmaDisable(interface->rxDma);

  reg->INTEN = 0;
  reg->FUNCSEL = FUNCSEL_TXRXDIS;
  while (reg->FIFOSTS & FIFOSTS_TXRXACT);

#ifdef CONFIG_PLATFORM_NUMICRO_UART_PM
  pmUnregister(interface);
#endif

  /* Free DMA channels */
  deinit(interface->txDma);
  deinit(interface->rxDma);

  if (interface->preallocated)
  {
    byteQueueDeinitArena(&interface->txQueue);
    byteQueueDeinitArena(&interface->rxQueue);
  }
  else
  {
    /* Free memory allocated for queues */
    byteQueueDeinit(&interface->txQueue);
    byteQueueDeinit(&interface->rxQueue);
  }

  /* Call base class destructor */
  UartBase->deinit(interface);
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

#ifdef CONFIG_PLATFORM_NUMICRO_UART_RC
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

#ifdef CONFIG_PLATFORM_NUMICRO_UART_WATERMARK
    case IF_RX_WATERMARK:
      *(size_t *)data = interface->rxWatermark;
      return E_OK;

    case IF_TX_WATERMARK:
      *(size_t *)data = interface->txWatermark;
      return E_OK;
#endif

#ifdef CONFIG_PLATFORM_NUMICRO_UART_RC
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

#ifdef CONFIG_PLATFORM_NUMICRO_UART_RC
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

#ifdef CONFIG_PLATFORM_NUMICRO_UART_PM
      interface->rate = rate;
#endif /* CONFIG_PLATFORM_NUMICRO_UART_PM */

      uartSetRate(&interface->base, rate);
      return E_OK;
    }

    default:
      return E_INVALID;
  }
#else /* CONFIG_PLATFORM_NUMICRO_UART_RC */
  (void)interface;
  (void)parameter;
  (void)data;

  return E_INVALID;
#endif /* CONFIG_PLATFORM_NUMICRO_UART_RC */
}
/*----------------------------------------------------------------------------*/
static size_t serialRead(void *object, void *buffer, size_t length)
{
  struct SerialDma * const interface = object;
  uint8_t *position = buffer;

  IrqState state = irqSave();
  readResidue(interface);
  irqRestore(state);

  while (length && !byteQueueEmpty(&interface->rxQueue))
  {
    const uint8_t *address;
    size_t count;

    byteQueueDeferredPop(&interface->rxQueue, &address, &count, 0);
    count = MIN(length, count);
    memcpy(position, address, count);

    state = irqSave();
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
    irqRestore(state);

    position += count;
    length -= count;
  }

  state = irqSave();
  if (!byteQueueEmpty(&interface->txQueue) && interface->txBufferSize == 0)
    enqueueTxBuffers(interface);
  irqRestore(state);

  return position - (const uint8_t *)buffer;
}