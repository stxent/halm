/*
 * serial_dma.c
 * Copyright (C) 2012 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <stdlib.h>
#include <string.h>
#include <halm/generic/byte_queue_extensions.h>
#include <halm/platform/nxp/gen_1/uart_defs.h>
#include <halm/platform/nxp/gpdma_circular.h>
#include <halm/platform/nxp/gpdma_oneshot.h>
#include <halm/platform/nxp/serial_dma.h>
#include <halm/pm.h>
/*----------------------------------------------------------------------------*/
#define RX_BUFFERS 2
/*----------------------------------------------------------------------------*/
static bool dmaSetup(struct SerialDma *, uint8_t, uint8_t);
static enum Result enqueueRxBuffers(struct SerialDma *);
static void rxDmaHandler(void *);
static void txDmaHandler(void *);
/*----------------------------------------------------------------------------*/
#ifdef CONFIG_PLATFORM_NXP_UART_PM
static void powerStateHandler(void *, enum PmState);
#endif
/*----------------------------------------------------------------------------*/
static enum Result serialInit(void *, const void *);
static enum Result serialSetCallback(void *, void (*)(void *), void *);
static enum Result serialGetParam(void *, enum IfParameter, void *);
static enum Result serialSetParam(void *, enum IfParameter, const void *);
static size_t serialRead(void *, void *, size_t);
static size_t serialWrite(void *, const void *, size_t);

#ifndef CONFIG_PLATFORM_NXP_UART_NO_DEINIT
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
    uint8_t txChannel)
{
  static const struct GpDmaSettings dmaSettings[] = {
      {
          .source = {
              .burst = DMA_BURST_1,
              .width = DMA_WIDTH_BYTE,
              .increment = false
          },
          .destination = {
              .burst = DMA_BURST_4,
              .width = DMA_WIDTH_BYTE,
              .increment = true
          }
      },
      {
          .source = {
              .burst = DMA_BURST_4,
              .width = DMA_WIDTH_BYTE,
              .increment = true
          },
          .destination = {
              .burst = DMA_BURST_8,
              .width = DMA_WIDTH_BYTE,
              .increment = false
          }
      }
  };
  const struct GpDmaCircularConfig rxDmaConfig = {
      .number = RX_BUFFERS,
      .event = GPDMA_UART0_RX + interface->base.channel,
      .type = GPDMA_TYPE_P2M,
      .channel = rxChannel,
      .silent = false
  };
  const struct GpDmaOneShotConfig txDmaConfig = {
      .event = GPDMA_UART0_TX + interface->base.channel,
      .type = GPDMA_TYPE_M2P,
      .channel = txChannel
  };

  interface->rxDma = init(GpDmaCircular, &rxDmaConfig);
  if (!interface->rxDma)
    return false;
  dmaConfigure(interface->rxDma, &dmaSettings[0]);
  dmaSetCallback(interface->rxDma, rxDmaHandler, interface);

  interface->txDma = init(GpDmaOneShot, &txDmaConfig);
  if (!interface->txDma)
    return false;
  dmaConfigure(interface->txDma, &dmaSettings[1]);
  dmaSetCallback(interface->txDma, txDmaHandler, interface);

  return true;
}
/*----------------------------------------------------------------------------*/
static enum Result enqueueRxBuffers(struct SerialDma *interface)
{
  const LPC_UART_Type * const reg = interface->base.reg;
  const void * const source = (const void *)&reg->RBR;
  uint8_t *destination = interface->rxBuffer;

  for (size_t index = 0; index < RX_BUFFERS; ++index)
  {
    dmaAppend(interface->rxDma, destination, source, interface->rxBufferSize);
    destination += interface->rxBufferSize;
  }

  /* Start reception */
  return dmaEnable(interface->rxDma);
}
/*----------------------------------------------------------------------------*/
static void rxDmaHandler(void *object)
{
  struct SerialDma * const interface = object;

  assert(dmaStatus(interface->rxDma) == E_BUSY);

  const size_t offset = interface->rxBufferIndex * interface->rxBufferSize;

  byteQueuePushArray(&interface->rxQueue, interface->rxBuffer + offset,
      interface->rxBufferSize);

  if (++interface->rxBufferIndex == RX_BUFFERS)
    interface->rxBufferIndex = 0;

  if (interface->callback)
    interface->callback(interface->callbackArgument);
}
/*----------------------------------------------------------------------------*/
static void txDmaHandler(void *object)
{
  struct SerialDma * const interface = object;

  byteQueueAbandon(&interface->txQueue, interface->txBufferSize);

  if (!byteQueueEmpty(&interface->txQueue))
  {
    LPC_UART_Type * const reg = interface->base.reg;
    void *queueChunk;

    byteQueueDeferredPop(&interface->txQueue, &queueChunk,
        &interface->txBufferSize);
    dmaAppend(interface->txDma, (void *)&reg->THR, queueChunk,
        interface->txBufferSize);

    const enum Result res = dmaEnable(interface->txDma);
    assert(res == E_OK);
    (void)res;
  }
  else
  {
    interface->txBufferSize = 0;
  }

  if (byteQueueEmpty(&interface->txQueue) && interface->callback)
    interface->callback(interface->callbackArgument);
}
/*----------------------------------------------------------------------------*/
#ifdef CONFIG_PLATFORM_NXP_UART_PM
static void powerStateHandler(void *object, enum PmState state)
{
  struct SerialDma * const interface = object;

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
  const struct SerialDmaConfig * const config = configBase;
  assert(config);

  const struct UartBaseConfig baseConfig = {
      .rx = config->rx,
      .tx = config->tx,
      .channel = config->channel
  };
  struct SerialDma * const interface = object;
  struct UartRateConfig rateConfig;
  enum Result res;

  /* Call base class constructor */
  if ((res = UartBase->init(object, &baseConfig)) != E_OK)
    return res;

  if ((res = uartCalcRate(object, config->rate, &rateConfig)) != E_OK)
    return res;

  /* Allocate ring buffer for reception */
  interface->rxBuffer = malloc(config->rxChunk * RX_BUFFERS);
  if (!interface->rxBuffer)
    return E_MEMORY;

  if (byteQueueInit(&interface->rxQueue, config->rxLength))
    return E_MEMORY;
  if (byteQueueInit(&interface->txQueue, config->txLength))
    return E_MEMORY;

  if (!dmaSetup(interface, config->dma[0], config->dma[1]))
    return E_ERROR;

  interface->callback = 0;
  interface->rate = config->rate;

  interface->rxBufferIndex = 0;
  interface->rxBufferSize = config->rxChunk;
  interface->txBufferSize = 0;

  LPC_UART_Type * const reg = interface->base.reg;

  /* Set 8-bit length */
  reg->LCR = LCR_WLS_8BIT;
  /* Enable FIFO and DMA, set RX trigger level */
  reg->FCR = (reg->FCR & ~FCR_RXTRIGLVL_MASK) | FCR_FIFOEN | FCR_DMAMODE
      | FCR_RXTRIGLVL(RX_TRIGGER_LEVEL_1);
  /* Disable all interrupts */
  reg->IER = 0;
  /* Transmitter is enabled by default */

  uartSetParity(object, config->parity);
  uartSetRate(object, rateConfig);

#ifdef CONFIG_PLATFORM_NXP_UART_PM
  if ((res = pmRegister(powerStateHandler, interface)) != E_OK)
    return res;
#endif

  if ((res = enqueueRxBuffers(interface)) != E_OK)
    return res;

  return E_OK;
}
/*----------------------------------------------------------------------------*/
#ifndef CONFIG_PLATFORM_NXP_UART_NO_DEINIT
static void serialDeinit(void *object)
{
  struct SerialDma * const interface = object;

  /* Stop channels */
  dmaDisable(interface->rxDma);

#ifdef CONFIG_PLATFORM_NXP_UART_PM
  pmUnregister(interface);
#endif

  /* Free DMA descriptors */
  deinit(interface->txDma);
  deinit(interface->rxDma);

  /* Free temporary buffers and queues */
  byteQueueDeinit(&interface->txQueue);
  byteQueueDeinit(&interface->rxQueue);
  free(interface->rxBuffer);

  /* Call base class destructor */
  UartBase->deinit(interface);
}
#endif
/*----------------------------------------------------------------------------*/
static enum Result serialSetCallback(void *object, void (*callback)(void *),
    void *argument)
{
  struct SerialDma * const interface = object;

  interface->callbackArgument = argument;
  interface->callback = callback;
  return E_OK;
}
/*----------------------------------------------------------------------------*/
static enum Result serialGetParam(void *object, enum IfParameter parameter,
    void *data __attribute__((unused)))
{
  struct SerialDma * const interface = object;

#ifdef CONFIG_PLATFORM_NXP_UART_RC
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

#ifdef CONFIG_PLATFORM_NXP_UART_RC
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
  struct SerialDma * const interface = object;

#ifdef CONFIG_PLATFORM_NXP_UART_RC
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
  struct SerialDma * const interface = object;

  irqDisable(GPDMA_IRQ);
  const size_t read = byteQueuePopArray(&interface->rxQueue, buffer, length);
  irqEnable(GPDMA_IRQ);

  return read;
}
/*----------------------------------------------------------------------------*/
static size_t serialWrite(void *object, const void *buffer, size_t length)
{
  struct SerialDma * const interface = object;

  if (!length)
    return 0;

  /*
   * Disable interrupts before status check because DMA interrupt
   * may be called and transmission will stall.
   */
  irqDisable(GPDMA_IRQ);

  const size_t written = byteQueuePushArray(&interface->txQueue,
      buffer, length);

  if (dmaStatus(interface->txDma) != E_BUSY)
  {
    LPC_UART_Type * const reg = interface->base.reg;
    void *queueChunk;

    byteQueueDeferredPop(&interface->txQueue, &queueChunk,
        &interface->txBufferSize);
    dmaAppend(interface->txDma, (void *)&reg->THR, queueChunk,
        interface->txBufferSize);

    const enum Result res = dmaEnable(interface->txDma);
    assert(res == E_OK);
    (void)res;
  }

  irqEnable(GPDMA_IRQ);

  return written;
}
