/*
 * serial_dma_toc.c
 * Copyright (C) 2023 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/platform/numicro/clocking.h>
#include <halm/platform/numicro/pdma_circular_toc.h>
#include <halm/platform/numicro/pdma_oneshot.h>
#include <halm/platform/numicro/serial_dma_toc.h>
#include <halm/platform/numicro/uart_base.h>
#include <halm/platform/numicro/uart_defs.h>
#include <halm/pm.h>
#include <xcore/containers/byte_queue.h>
#include <stdlib.h>
#include <string.h>
/*----------------------------------------------------------------------------*/
struct SerialDmaTOC
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
  /* Pointer to the temporary reception buffer */
  uint8_t *rxBuffer;

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
static bool dmaSetup(struct SerialDmaTOC *, uint8_t, uint8_t, uint32_t);
static enum Result enqueueRxBuffer(struct SerialDmaTOC *);
static enum Result enqueueTxBuffers(struct SerialDmaTOC *);
static bool readResidue(struct SerialDmaTOC *);
static void rxDmaHandler(void *);
static void txDmaHandler(void *);
static void updateRxWatermark(struct SerialDmaTOC *, size_t);
static void updateTxWatermark(struct SerialDmaTOC *, size_t);

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
#  define serialDeinit deletedDestructorTrap
#endif
/*----------------------------------------------------------------------------*/
const struct InterfaceClass * const SerialDmaTOC = &(const struct InterfaceClass){
    .size = sizeof(struct SerialDmaTOC),
    .init = serialInit,
    .deinit = serialDeinit,

    .setCallback = serialSetCallback,
    .getParam = serialGetParam,
    .setParam = serialSetParam,
    .read = serialRead,
    .write = serialWrite
};

/*----------------------------------------------------------------------------*/
static bool dmaSetup(struct SerialDmaTOC *interface, uint8_t channelA,
    uint8_t channelB, uint32_t timeout)
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

  uint8_t rxChannel;
  uint8_t txChannel;

  if (pdmaIsTOCAvailable(channelA))
  {
    rxChannel = channelA;
    txChannel = channelB;
  }
  else if (pdmaIsTOCAvailable(channelB))
  {
    rxChannel = channelB;
    txChannel = channelA;
  }
  else
    return false;

  const struct PdmaCircularTOCConfig rxDmaConfig = {
      .number = 2,
      .timeout = timeout,
      .event = pdmaGetEventUartRx(interface->base.channel),
      .channel = rxChannel,
      .oneshot = false,
      .silent = false
  };
  const struct PdmaOneShotConfig txDmaConfig = {
      .event = pdmaGetEventUartTx(interface->base.channel),
      .channel = txChannel
  };

  interface->rxDma = init(PdmaCircularTOC, &rxDmaConfig);
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
static enum Result enqueueRxBuffer(struct SerialDmaTOC *interface)
{
  const NM_UART_Type * const reg = interface->base.reg;
  const size_t chunk = interface->rxBufferSize >> 1;

  dmaAppend(interface->rxDma, interface->rxBuffer,
      (const void *)&reg->DAT, chunk);
  dmaAppend(interface->rxDma, interface->rxBuffer + chunk,
      (const void *)&reg->DAT, chunk);
  interface->rxPosition = 0;

  /* Start reception */
  return dmaEnable(interface->rxDma);
}
/*----------------------------------------------------------------------------*/
static enum Result enqueueTxBuffers(struct SerialDmaTOC *interface)
{
  NM_UART_Type * const reg = interface->base.reg;
  const uint8_t *address;
  enum Result res;

  byteQueueDeferredPop(&interface->txQueue, &address,
      &interface->txBufferSize, 0);
  dmaAppend(interface->txDma, (void *)&reg->DAT, address,
      interface->txBufferSize);

  if ((res = dmaEnable(interface->txDma)) != E_OK)
    interface->txBufferSize = 0;

  return res;
}
/*----------------------------------------------------------------------------*/
static bool readResidue(struct SerialDmaTOC *interface)
{
  const size_t chunk = interface->rxBufferSize >> 1;
  size_t residue;

  if (dmaResidue(interface->rxDma, &residue) != E_OK)
    return false;
  if (interface->rxPosition < chunk)
    residue += chunk;

  const size_t pending =
      interface->rxBufferSize - interface->rxPosition - residue;

  if (pending)
  {
    byteQueuePushArray(&interface->rxQueue,
        interface->rxBuffer + interface->rxPosition, pending);
    interface->rxPosition += pending;
  }

  return true;
}
/*----------------------------------------------------------------------------*/
static void rxDmaHandler(void *object)
{
  struct SerialDmaTOC * const interface = object;

  const enum Result status = dmaStatus(interface->rxDma);
  assert(status == E_BUSY || status == E_TIMEOUT);

  if (status == E_BUSY || !readResidue(interface))
  {
    const size_t index = dmaQueued(interface->rxDma);
    assert(index >= 1 && index <= 2);

    const size_t end = interface->rxBufferSize >> (2 - index);
    const size_t pending = end - interface->rxPosition;

    byteQueuePushArray(&interface->rxQueue,
        interface->rxBuffer + interface->rxPosition, pending);
    interface->rxPosition = index == 1 ? (interface->rxBufferSize >> 1) : 0;
  }

  updateRxWatermark(interface, byteQueueSize(&interface->rxQueue));

  if (interface->callback != NULL)
    interface->callback(interface->callbackArgument);
}
/*----------------------------------------------------------------------------*/
static void txDmaHandler(void *object)
{
  struct SerialDmaTOC * const interface = object;
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
static void updateRxWatermark(struct SerialDmaTOC *interface, size_t level)
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
static void updateTxWatermark(struct SerialDmaTOC *interface, size_t level)
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
  if (state == PM_ACTIVE)
  {
    struct SerialDmaTOC * const interface = object;
    uartSetRate(&interface->base, interface->rate);
  }
}
#endif
/*----------------------------------------------------------------------------*/
static enum Result serialInit(void *object, const void *configBase)
{
  const struct SerialDmaTOCConfig * const config = configBase;
  assert(config != NULL);
  assert(config->rxChunk > 0 && config->rxChunk % 2 == 0);
  assert(config->rxLength > 0 && config->txLength > 0);
  assert(config->rxLength % config->rxChunk == 0);

  const struct UartBaseConfig baseConfig = {
      .rx = config->rx,
      .tx = config->tx,
      .priority = config->priority,
      .channel = config->channel
  };
  struct SerialDmaTOC * const interface = object;
  enum Result res;

  /* Call base class constructor */
  if ((res = UartBase->init(interface, &baseConfig)) != E_OK)
    return res;

  if (!byteQueueInit(&interface->rxQueue, config->rxLength))
    return E_MEMORY;
  if (!byteQueueInit(&interface->txQueue, config->txLength))
    return E_MEMORY;

  /* Allocate ring buffer for reception */
  interface->rxBuffer = malloc(config->rxChunk);
  if (!interface->rxBuffer)
    return E_MEMORY;

  const uint32_t ahbClock = clockFrequency(MainClock);
  const uint32_t timeout = (ahbClock / config->rate) * config->timeout;

  if (!dmaSetup(interface, config->dma[0], config->dma[1], timeout))
    return E_ERROR;

  interface->callback = 0;
  interface->rxBufferSize = config->rxChunk;
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

  if (!uartSetRate(&interface->base, config->rate))
    return E_VALUE;
  uartSetParity(&interface->base, config->parity);

  reg->FUNCSEL = FUNCSEL_FUNCSEL(FUNCSEL_UART);
  reg->INTEN = INTEN_TXPDMAEN | INTEN_RXPDMAEN;

#ifdef CONFIG_PLATFORM_NUMICRO_UART_PM
  interface->rate = config->rate;

  if ((res = pmRegister(powerStateHandler, interface)) != E_OK)
    return res;
#endif

  return enqueueRxBuffer(interface);
}
/*----------------------------------------------------------------------------*/
#ifndef CONFIG_PLATFORM_NUMICRO_UART_NO_DEINIT
static void serialDeinit(void *object)
{
  struct SerialDmaTOC * const interface = object;
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

  free(interface->rxBuffer);
  byteQueueDeinit(&interface->txQueue);
  byteQueueDeinit(&interface->rxQueue);

  /* Call base class destructor */
  UartBase->deinit(interface);
}
#endif
/*----------------------------------------------------------------------------*/
static void serialSetCallback(void *object, void (*callback)(void *),
    void *argument)
{
  struct SerialDmaTOC * const interface = object;

  interface->callbackArgument = argument;
  interface->callback = callback;
}
/*----------------------------------------------------------------------------*/
static enum Result serialGetParam(void *object, int parameter, void *data)
{
  struct SerialDmaTOC * const interface = object;

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
  struct SerialDmaTOC * const interface = object;

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

      if (uartSetRate(&interface->base, rate))
      {
#  ifdef CONFIG_PLATFORM_NUMICRO_UART_PM
        interface->rate = rate;
#  endif /* CONFIG_PLATFORM_NUMICRO_UART_PM */

        return E_OK;
      }
      else
        return E_VALUE;
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
  struct SerialDmaTOC * const interface = object;
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
  struct SerialDmaTOC * const interface = object;
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
  updateTxWatermark(interface, byteQueueSize(&interface->txQueue));
  if (!byteQueueEmpty(&interface->txQueue) && interface->txBufferSize == 0)
    enqueueTxBuffers(interface);
  irqRestore(state);

  return position - (const uint8_t *)buffer;
}
