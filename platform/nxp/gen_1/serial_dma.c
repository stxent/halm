/*
 * serial_dma.c
 * Copyright (C) 2012 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <stdlib.h>
#include <string.h>
#include <halm/platform/nxp/gen_1/uart_defs.h>
#include <halm/platform/nxp/gpdma.h>
#include <halm/platform/nxp/gpdma_circular.h>
#include <halm/platform/nxp/serial_dma.h>
#include <halm/pm.h>
/*----------------------------------------------------------------------------*/
/*
 * Size of temporary buffers should be increased if the baud rate
 * is higher than 500 kbit/s.
 */
#define BUFFER_SIZE 16
#define RX_BUFFERS  2
/*----------------------------------------------------------------------------*/
static enum result dmaSetup(struct SerialDma *, uint8_t, uint8_t);
static enum result enqueueRxBuffers(struct SerialDma *);
static void rxDmaHandler(void *);
static void txDmaHandler(void *);
/*----------------------------------------------------------------------------*/
#ifdef CONFIG_PLATFORM_NXP_UART_PM
static void powerStateHandler(void *, enum pmState);
#endif
/*----------------------------------------------------------------------------*/
static enum result serialInit(void *, const void *);
static void serialDeinit(void *);
static enum result serialCallback(void *, void (*)(void *), void *);
static enum result serialGet(void *, enum ifOption, void *);
static enum result serialSet(void *, enum ifOption, const void *);
static size_t serialRead(void *, void *, size_t);
static size_t serialWrite(void *, const void *, size_t);
/*----------------------------------------------------------------------------*/
static const struct InterfaceClass serialTable = {
    .size = sizeof(struct SerialDma),
    .init = serialInit,
    .deinit = serialDeinit,

    .callback = serialCallback,
    .get = serialGet,
    .set = serialSet,
    .read = serialRead,
    .write = serialWrite
};
/*----------------------------------------------------------------------------*/
const struct InterfaceClass * const SerialDma = &serialTable;
/*----------------------------------------------------------------------------*/
static enum result dmaSetup(struct SerialDma *interface, uint8_t rxChannel,
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
  const struct GpDmaConfig txDmaConfig = {
      .event = GPDMA_UART0_TX + interface->base.channel,
      .type = GPDMA_TYPE_M2P,
      .channel = txChannel
  };

  interface->rxDma = init(GpDmaCircular, &rxDmaConfig);
  if (!interface->rxDma)
    return E_ERROR;
  dmaConfigure(interface->rxDma, &dmaSettings[0]);
  dmaCallback(interface->rxDma, rxDmaHandler, interface);

  interface->txDma = init(GpDma, &txDmaConfig);
  if (!interface->txDma)
    return E_ERROR;
  dmaConfigure(interface->txDma, &dmaSettings[1]);
  dmaCallback(interface->txDma, txDmaHandler, interface);

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static enum result enqueueRxBuffers(struct SerialDma *interface)
{
  const LPC_UART_Type * const reg = interface->base.reg;
  const void * const source = (const void *)&reg->RBR;
  uint8_t *destination = interface->rxBuffer;

  for (size_t index = 0; index < RX_BUFFERS; ++index)
  {
    dmaAppend(interface->rxDma, destination, source, BUFFER_SIZE);
    destination += BUFFER_SIZE;
  }

  /* Start reception */
  return dmaEnable(interface->rxDma);
}
/*----------------------------------------------------------------------------*/
static void rxDmaHandler(void *object)
{
  struct SerialDma * const interface = object;

  assert(dmaStatus(interface->rxDma) == E_BUSY);

  byteQueuePushArray(&interface->rxQueue, interface->rxBuffer
      + interface->rxBufferIndex * BUFFER_SIZE, BUFFER_SIZE);

  if (++interface->rxBufferIndex == RX_BUFFERS)
    interface->rxBufferIndex = 0;

  if (interface->callback)
    interface->callback(interface->callbackArgument);
}
/*----------------------------------------------------------------------------*/
static void txDmaHandler(void *object)
{
  struct SerialDma * const interface = object;

  if (!byteQueueEmpty(&interface->txQueue))
  {
    const size_t length = byteQueuePopArray(&interface->txQueue,
        interface->txBuffer, BUFFER_SIZE);
    LPC_UART_Type * const reg = interface->base.reg;

    dmaAppend(interface->txDma, (void *)&reg->THR, interface->txBuffer, length);

    const enum result res = dmaEnable(interface->txDma);
    assert(res == E_OK);
    (void)res;
  }

  if (byteQueueEmpty(&interface->txQueue) && interface->callback)
    interface->callback(interface->callbackArgument);
}
/*----------------------------------------------------------------------------*/
#ifdef CONFIG_PLATFORM_NXP_UART_PM
static void powerStateHandler(void *object, enum pmState state)
{
  struct SerialDma * const interface = object;

  if (state == PM_ACTIVE)
  {
    struct UartRateConfig rateConfig;

    /* Recalculate and set baud rate */
    if (uartCalcRate(object, interface->rate, &rateConfig))
      uartSetRate(object, rateConfig);
  }
}
#endif
/*----------------------------------------------------------------------------*/
static enum result serialInit(void *object, const void *configBase)
{
  const struct SerialDmaConfig * const config = configBase;
  const struct UartBaseConfig baseConfig = {
      .channel = config->channel,
      .rx = config->rx,
      .tx = config->tx
  };
  struct SerialDma * const interface = object;
  struct UartRateConfig rateConfig;
  enum result res;

  /* Call base class constructor */
  if ((res = UartBase->init(object, &baseConfig)) != E_OK)
    return res;

  if ((res = uartCalcRate(object, config->rate, &rateConfig)) != E_OK)
    return res;

  if ((res = byteQueueInit(&interface->rxQueue, config->rxLength)) != E_OK)
    return res;
  if ((res = byteQueueInit(&interface->txQueue, config->txLength)) != E_OK)
    return res;

  if ((res = dmaSetup(interface, config->dma[0], config->dma[1])) != E_OK)
    return res;

  /* Allocate one buffer for transmission and multiple buffers for reception */
  interface->pool = malloc(BUFFER_SIZE * (1 + RX_BUFFERS));
  interface->txBuffer = interface->pool;
  interface->rxBuffer = interface->pool + BUFFER_SIZE;

  interface->callback = 0;
  interface->rate = config->rate;

  interface->rxBufferIndex = 0;

  LPC_UART_Type * const reg = interface->base.reg;

  /* Set 8-bit length */
  reg->LCR = LCR_WORD_8BIT;
  /* Enable FIFO and DMA, set RX trigger level */
  reg->FCR = (reg->FCR & ~FCR_RX_TRIGGER_MASK) | FCR_ENABLE | FCR_DMA_ENABLE
      | FCR_RX_TRIGGER(RX_TRIGGER_LEVEL_1);
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
  free(interface->pool);
  byteQueueDeinit(&interface->txQueue);
  byteQueueDeinit(&interface->rxQueue);

  /* Call base class destructor */
  UartBase->deinit(interface);
}
/*----------------------------------------------------------------------------*/
static enum result serialCallback(void *object, void (*callback)(void *),
    void *argument)
{
  struct SerialDma * const interface = object;

  interface->callbackArgument = argument;
  interface->callback = callback;
  return E_OK;
}
/*----------------------------------------------------------------------------*/
static enum result serialGet(void *object, enum ifOption option,
    void *data __attribute__((unused)))
{
  struct SerialDma * const interface = object;

  switch (option)
  {
    case IF_AVAILABLE:
      *(size_t *)data = byteQueueSize(&interface->rxQueue);
      return E_OK;

    case IF_PENDING:
      *(size_t *)data = byteQueueSize(&interface->txQueue);
      return E_OK;

    default:
      return E_INVALID;
  }
}
/*----------------------------------------------------------------------------*/
static enum result serialSet(void *object, enum ifOption option,
    const void *data)
{
  struct SerialDma * const interface = object;

  switch (option)
  {
    case IF_RATE:
    {
      struct UartRateConfig rateConfig;
      const enum result res = uartCalcRate(object, *(const uint32_t *)data,
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
}
/*----------------------------------------------------------------------------*/
static size_t serialRead(void *object, void *buffer, size_t length)
{
  struct SerialDma * const interface = object;

  const irqState state = irqSave();
  const size_t read = byteQueuePopArray(&interface->rxQueue, buffer, length);
  irqRestore(state);

  return read;
}
/*----------------------------------------------------------------------------*/
static size_t serialWrite(void *object, const void *buffer, size_t length)
{
  struct SerialDma * const interface = object;
  const uint8_t *bufferPosition = buffer;
  const size_t initialLength = length;
  size_t chunkLength = 0;

  /*
   * Disable interrupts before status check because DMA interrupt
   * may be called and transmission will stall.
   */
  const irqState state = irqSave();

  if (dmaStatus(interface->txDma) != E_BUSY)
  {
    chunkLength = length < BUFFER_SIZE ? length : BUFFER_SIZE;

    memcpy(interface->txBuffer, bufferPosition, chunkLength);

    length -= chunkLength;
    bufferPosition += chunkLength;
  }
  length -= byteQueuePushArray(&interface->txQueue, bufferPosition, length);

  irqRestore(state);

  if (chunkLength)
  {
    LPC_UART_Type * const reg = interface->base.reg;

    dmaAppend(interface->txDma, (void *)&reg->THR, interface->txBuffer,
        chunkLength);

    if (dmaEnable(interface->txDma) != E_OK)
    {
      dmaClear(interface->txDma);
      return 0;
    }
  }

  return initialLength - length;
}
