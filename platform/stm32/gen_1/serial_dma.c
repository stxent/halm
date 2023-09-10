/*
 * serial_dma.c
 * Copyright (C) 2021 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/generic/byte_queue_extensions.h>
#include <halm/platform/stm32/dma_circular.h>
#include <halm/platform/stm32/dma_oneshot.h>
#include <halm/platform/stm32/serial_dma.h>
#include <halm/platform/stm32/uart_base.h>
#include <halm/platform/stm32/uart_defs.h>
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
  /* Pointer to the temporary reception buffer */
  uint8_t *rxBuffer;
  /* Position inside the temporary buffer */
  size_t rxPosition;

  /* Size of the circular reception buffer */
  size_t rxBufferSize;
  /* Size of the DMA TX transfer */
  size_t txBufferSize;

#ifdef CONFIG_PLATFORM_STM32_UART_PM
  /* Desired baud rate */
  uint32_t rate;
#endif

#ifdef CONFIG_PLATFORM_STM32_UART_WATERMARK
  /* Maximum available frames in the receive queue */
  size_t rxWatermark;
  /* Maximum pending frames in the transmit queue */
  size_t txWatermark;
#endif
};
/*----------------------------------------------------------------------------*/
static bool dmaSetup(struct SerialDma *, uint8_t, uint8_t);
static enum Result enqueueRxBuffer(struct SerialDma *);
static enum Result enqueueTxBuffers(struct SerialDma *);
static void rxDmaHandler(void *);
static void serialInterruptHandler(void *);
static void txDmaHandler(void *);
static void updateRxWatermark(struct SerialDma *, size_t);
static void updateTxWatermark(struct SerialDma *, size_t);

#ifdef CONFIG_PLATFORM_STM32_UART_PM
static void powerStateHandler(void *, enum PmState);
#endif
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
static bool dmaSetup(struct SerialDma *interface, uint8_t rxStream,
    uint8_t txStream)
{
  static const struct DmaSettings dmaSettings[] = {
      {
          .source = {
              .burst = DMA_BURST_1,
              .width = DMA_WIDTH_BYTE,
              .increment = false
          },
          .destination = {
              .burst = DMA_BURST_1,
              .width = DMA_WIDTH_BYTE,
              .increment = true
          }
      }, {
          .source = {
              .burst = DMA_BURST_1,
              .width = DMA_WIDTH_BYTE,
              .increment = true
          },
          .destination = {
              .burst = DMA_BURST_1,
              .width = DMA_WIDTH_BYTE,
              .increment = false
          }
      }
  };
  const struct DmaCircularConfig rxDmaConfig = {
      .type = DMA_TYPE_P2M,
      .priority = 0,
      .stream = rxStream
  };
  const struct DmaOneShotConfig txDmaConfig = {
      .type = DMA_TYPE_M2P,
      .priority = 0,
      .stream = txStream
  };

  interface->rxDma = init(DmaCircular, &rxDmaConfig);
  if (interface->rxDma == NULL)
    return false;
  dmaConfigure(interface->rxDma, &dmaSettings[0]);
  dmaSetCallback(interface->rxDma, rxDmaHandler, interface);

  interface->txDma = init(DmaOneShot, &txDmaConfig);
  if (interface->txDma == NULL)
    return false;
  dmaConfigure(interface->txDma, &dmaSettings[1]);
  dmaSetCallback(interface->txDma, txDmaHandler, interface);

  return true;
}
/*----------------------------------------------------------------------------*/
static enum Result enqueueRxBuffer(struct SerialDma *interface)
{
  const STM_USART_Type * const reg = interface->base.reg;

  dmaAppend(interface->rxDma, interface->rxBuffer, (const void *)&reg->DR,
      interface->rxBufferSize);
  interface->rxPosition = 0;

  /* Start reception */
  return dmaEnable(interface->rxDma);
}
/*----------------------------------------------------------------------------*/
static enum Result enqueueTxBuffers(struct SerialDma *interface)
{
  STM_USART_Type * const reg = interface->base.reg;
  const uint8_t *address;
  enum Result res;

  byteQueueDeferredPop(&interface->txQueue, &address,
      &interface->txBufferSize, 0);
  dmaAppend(interface->txDma, (void *)&reg->DR, address,
      interface->txBufferSize);

  if ((res = dmaEnable(interface->txDma)) != E_OK)
    interface->txBufferSize = 0;

  return res;
}
/*----------------------------------------------------------------------------*/
static void rxDmaHandler(void *object)
{
  struct SerialDma * const interface = object;
  const size_t index = dmaQueued(interface->rxDma);

  assert(index >= 1 && index <= 2);
  assert(dmaStatus(interface->rxDma) == E_BUSY);

  const size_t end = interface->rxBufferSize >> (2 - index);
  const size_t count = end - interface->rxPosition;

  byteQueuePushArray(&interface->rxQueue,
      interface->rxBuffer + interface->rxPosition, count);
  interface->rxPosition = index == 1 ? (interface->rxBufferSize >> 1) : 0;

  updateRxWatermark(interface, byteQueueSize(&interface->rxQueue));

  if (interface->callback != NULL)
    interface->callback(interface->callbackArgument);
}
/*----------------------------------------------------------------------------*/
static void serialInterruptHandler(void *object)
{
  struct SerialDma * const interface = object;
  STM_USART_Type * const reg = interface->base.reg;

  assert(dmaStatus(interface->rxDma) == E_BUSY);

  /* Clear IDLE flag */
  (void)reg->SR;
  (void)reg->DR;

  size_t residue;

  if (dmaResidue(interface->rxDma, &residue) == E_OK)
  {
    const size_t pending =
        interface->rxBufferSize - interface->rxPosition - residue;

    if (pending)
    {
      byteQueuePushArray(&interface->rxQueue,
          interface->rxBuffer + interface->rxPosition, pending);
      interface->rxPosition += pending;

      updateRxWatermark(interface, byteQueueSize(&interface->rxQueue));

      if (interface->callback != NULL)
        interface->callback(interface->callbackArgument);
    }
  }
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

  if (event && interface->callback != NULL)
    interface->callback(interface->callbackArgument);
}
/*----------------------------------------------------------------------------*/
static void updateRxWatermark(struct SerialDma *interface, size_t level)
{
#ifdef CONFIG_PLATFORM_STM32_UART_WATERMARK
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
#ifdef CONFIG_PLATFORM_STM32_UART_WATERMARK
  if (level > interface->txWatermark)
    interface->txWatermark = level;
#else
  (void)interface;
  (void)level;
#endif
}
/*----------------------------------------------------------------------------*/
#ifdef CONFIG_PLATFORM_STM32_UART_PM
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

  const struct UartBaseConfig baseConfig = {
      .rx = config->rx,
      .tx = config->tx,
      .channel = config->channel
  };
  struct SerialDma * const interface = object;
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
  if (interface->rxBuffer == NULL)
    return E_MEMORY;

  interface->base.handler = serialInterruptHandler;
  interface->callback = NULL;
  interface->rxBufferSize = config->rxChunk;
  interface->txBufferSize = 0;

#ifdef CONFIG_PLATFORM_STM32_UART_WATERMARK
  interface->rxWatermark = 0;
  interface->txWatermark = 0;
#endif

  if (!dmaSetup(interface, config->rxDma, config->txDma))
    return E_ERROR;

  STM_USART_Type * const reg = interface->base.reg;

  /* Disable the peripheral */
  reg->CR1 = 0;

  uartSetRate(object, config->rate);
  uartSetParity(object, config->parity);

  /* Enable DMA mode for transmission and reception */
  reg->CR3 = CR3_DMAR | CR3_DMAT;

  /* Enable receiver and transmitter, IDLE interrupt, enable peripheral */
  reg->CR1 |= CR1_RE | CR1_TE | CR1_IDLEIE | CR1_UE;

#ifdef CONFIG_PLATFORM_STM32_UART_PM
  interface->rate = config->rate;

  if ((res = pmRegister(powerStateHandler, interface)) != E_OK)
    return res;
#endif

  irqSetPriority(interface->base.irq, CONFIG_PLATFORM_STM32_DMA_PRIORITY);
  irqEnable(interface->base.irq);

  return enqueueRxBuffer(interface);
}
/*----------------------------------------------------------------------------*/
#ifndef CONFIG_PLATFORM_STM32_UART_NO_DEINIT
static void serialDeinit(void *object)
{
  struct SerialDma * const interface = object;
  STM_USART_Type * const reg = interface->base.reg;

  /* Stop DMA channels */
  dmaDisable(interface->txDma);
  dmaDisable(interface->rxDma);

  reg->CR1 = 0;

#ifdef CONFIG_PLATFORM_STM32_UART_PM
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
  struct SerialDma * const interface = object;

  interface->callbackArgument = argument;
  interface->callback = callback;
}
/*----------------------------------------------------------------------------*/
static enum Result serialGetParam(void *object, int parameter, void *data)
{
  struct SerialDma * const interface = object;

#ifdef CONFIG_PLATFORM_STM32_UART_RC
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

#ifdef CONFIG_PLATFORM_STM32_UART_WATERMARK
    case IF_RX_WATERMARK:
      *(size_t *)data = interface->rxWatermark;
      return E_OK;

    case IF_TX_WATERMARK:
      *(size_t *)data = interface->txWatermark;
      return E_OK;
#endif

#ifdef CONFIG_PLATFORM_STM32_UART_RC
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

#ifdef CONFIG_PLATFORM_STM32_UART_RC
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

#  ifdef CONFIG_PLATFORM_STM32_UART_PM
      interface->rate = rate;
#  endif /* CONFIG_PLATFORM_STM32_UART_PM */

      uartSetRate(&interface->base, rate);
      return E_OK;
    }

    default:
      return E_INVALID;
  }
#else /* CONFIG_PLATFORM_STM32_UART_RC */
  (void)interface;
  (void)parameter;
  (void)data;

  return E_INVALID;
#endif /* CONFIG_PLATFORM_STM32_UART_RC */
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
