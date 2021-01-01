/*
 * serial_dma.c
 * Copyright (C) 2021 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/generic/byte_queue_extensions.h>
#include <halm/platform/stm32/gen_1/uart_defs.h>
#include <halm/platform/stm32/serial_dma.h>
#include <stdlib.h>
#include <string.h>
/*----------------------------------------------------------------------------*/
static bool dmaSetup(struct SerialDma *, uint8_t, uint8_t);
static enum Result enqueueRxBuffer(struct SerialDma *);
static enum Result enqueueTxBuffers(struct SerialDma *);
static void rxDmaHandler(void *);
static void serialInterruptHandler(void *);
static void txDmaHandler(void *);
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
              .width = DMA_WIDTH_BYTE,
              .increment = false
          },
          .destination = {
              .width = DMA_WIDTH_BYTE,
              .increment = true
          }
      }, {
          .source = {
              .width = DMA_WIDTH_BYTE,
              .increment = true
          },
          .destination = {
              .width = DMA_WIDTH_BYTE,
              .increment = false
          }
      }
  };
  const struct DmaCircularConfig rxDmaConfig = {
      .type = DMA_TYPE_P2M,
      .stream = rxStream,
      .priority = 0
  };
  const struct DmaOneShotConfig txDmaConfig = {
      .type = DMA_TYPE_M2P,
      .stream = txStream,
      .priority = 0
  };

  interface->rxDma = init(DmaCircular, &rxDmaConfig);
  if (!interface->rxDma)
    return false;
  dmaConfigure(interface->rxDma, &dmaSettings[0]);
  dmaSetCallback(interface->rxDma, rxDmaHandler, interface);

  interface->txDma = init(DmaOneShot, &txDmaConfig);
  if (!interface->txDma)
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

  assert(dmaStatus(interface->rxDma) == E_BUSY);

  const size_t index = dmaPending(interface->rxDma);
  const size_t end = interface->rxBufferSize >> index;
  const size_t count = end - interface->rxPosition;

  byteQueuePushArray(&interface->rxQueue,
      interface->rxBuffer + interface->rxPosition, count);
  interface->rxPosition = index ? (interface->rxBufferSize >> 1) : 0;

  if (interface->callback)
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

  const size_t position =
      interface->rxBufferSize - dmaResidue(interface->rxDma);
  const size_t count = position - interface->rxPosition;

  if (count)
  {
    byteQueuePushArray(&interface->rxQueue,
        interface->rxBuffer + interface->rxPosition, count);
    interface->rxPosition += count;

    if (interface->callback)
      interface->callback(interface->callbackArgument);
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

  if (event && interface->callback)
    interface->callback(interface->callbackArgument);
}
/*----------------------------------------------------------------------------*/
static enum Result serialInit(void *object, const void *configBase)
{
  const struct SerialDmaConfig * const config = configBase;
  assert(config);

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

  /* Allocate ring buffer for reception */
  interface->rxBuffer = malloc(config->rxChunk);
  if (!interface->rxBuffer)
    return E_MEMORY;
  interface->rxBufferSize = config->rxChunk;
  interface->txBufferSize = 0;

  interface->base.handler = serialInterruptHandler;
  interface->callback = 0;
  interface->rate = config->rate;

  if (!byteQueueInit(&interface->rxQueue, config->rxLength))
    return E_MEMORY;
  if (!byteQueueInit(&interface->txQueue, config->txLength))
    return E_MEMORY;

  if (!dmaSetup(interface, config->rxDma, config->txDma))
    return E_ERROR;

  STM_USART_Type * const reg = interface->base.reg;

  /* Disable the peripheral */
  reg->CR1 = 0;

  uartSetRate(object, config->rate);
  uartSetParity(object, config->parity);

  /* Enable DMA mode for transmission and reception */
  reg->CR3 = CR3_DMAR | CR3_DMAT;

  /* Enable receiver and transmitter, IDLE interrupt, enable peripheral. */
  reg->CR1 |= CR1_RE | CR1_TE | CR1_IDLEIE | CR1_UE;

  irqSetPriority(interface->base.irq, config->priority);
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

  /* Free DMA channels */
  deinit(interface->txDma);
  deinit(interface->rxDma);

  reg->CR1 = 0;

  byteQueueDeinit(&interface->txQueue);
  byteQueueDeinit(&interface->rxQueue);
  free(interface->rxBuffer);

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
  struct SerialDma * const interface = object;

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
  if (!byteQueueEmpty(&interface->txQueue) && interface->txBufferSize == 0)
    enqueueTxBuffers(interface);
  irqRestore(state);

  return position - (const uint8_t *)buffer;
}
