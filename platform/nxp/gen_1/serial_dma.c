/*
 * serial_dma.c
 * Copyright (C) 2012 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <stdlib.h>
#include <string.h>
#include <pm.h>
#include <platform/nxp/gpdma.h>
#include <platform/nxp/gpdma_list.h>
#include <platform/nxp/serial_dma.h>
#include <platform/nxp/gen_1/uart_defs.h>
/*----------------------------------------------------------------------------*/
/*
 * Size of temporary buffers should be increased if the baud rate
 * is higher than 500 kbit/s.
 */
#define SINGLE_BUFFER_SIZE 16
#define RECEPTION_BUFFERS  3
/*----------------------------------------------------------------------------*/
static void rxDmaHandler(void *);
static void txDmaHandler(void *);
static enum result dmaSetup(struct SerialDma *, uint8_t, uint8_t);
/*----------------------------------------------------------------------------*/
#ifdef CONFIG_SERIAL_PM
static enum result powerStateHandler(void *, enum pmState);
#endif
/*----------------------------------------------------------------------------*/
static enum result serialInit(void *, const void *);
static void serialDeinit(void *);
static enum result serialCallback(void *, void (*)(void *), void *);
static enum result serialGet(void *, enum ifOption, void *);
static enum result serialSet(void *, enum ifOption, const void *);
static uint32_t serialRead(void *, uint8_t *, uint32_t);
static uint32_t serialWrite(void *, const uint8_t *, uint32_t);
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
static void rxDmaHandler(void *object)
{
  struct SerialDma * const interface = object;
  LPC_UART_Type * const reg = interface->base.reg;

  byteQueuePushArray(&interface->rxQueue, interface->rxBuffer
      + interface->rxBufferIndex * SINGLE_BUFFER_SIZE, SINGLE_BUFFER_SIZE);
  dmaStart(interface->rxDma, interface->rxBuffer + interface->rxBufferIndex
      * SINGLE_BUFFER_SIZE, (const void *)&reg->RBR, SINGLE_BUFFER_SIZE);

  if (++interface->rxBufferIndex == RECEPTION_BUFFERS)
    interface->rxBufferIndex = 0;

  if (interface->callback)
  {
    const uint32_t capacity = byteQueueCapacity(&interface->rxQueue);
    const uint32_t available = byteQueueSize(&interface->rxQueue);

    if (available >= (capacity >> 1))
      interface->callback(interface->callbackArgument);
  }
}
/*----------------------------------------------------------------------------*/
static void txDmaHandler(void *object)
{
  struct SerialDma * const interface = object;
  LPC_UART_Type * const reg = interface->base.reg;

  if (!byteQueueEmpty(&interface->txQueue))
  {
    const uint32_t chunkLength = byteQueuePopArray(&interface->txQueue,
        interface->txBuffer, SINGLE_BUFFER_SIZE);

    dmaStart(interface->txDma, (void *)&reg->THR, interface->txBuffer,
        chunkLength);
  }

  if (interface->callback)
  {
    const uint32_t capacity = byteQueueCapacity(&interface->txQueue);
    const uint32_t left = byteQueueSize(&interface->txQueue);

    if (left < (capacity >> 1))
      interface->callback(interface->callbackArgument);
  }
}
/*----------------------------------------------------------------------------*/
static enum result dmaSetup(struct SerialDma *interface, uint8_t rxChannel,
    uint8_t txChannel)
{
  const struct GpDmaListConfig rxChannelConfig = {
      .number = RECEPTION_BUFFERS,
      .size = SINGLE_BUFFER_SIZE,
      .channel = rxChannel,
      .event = GPDMA_UART0_RX + interface->base.channel,
      .source.increment = false,
      .destination.increment = true,
      .type = GPDMA_TYPE_P2M,
      .burst = DMA_BURST_1,
      .width = DMA_WIDTH_BYTE,
      .silent = false
  };
  const struct GpDmaConfig txChannelConfig = {
      .event = GPDMA_UART0_TX + interface->base.channel,
      .channel = txChannel,
      .source.increment = true,
      .destination.increment = false,
      .type = GPDMA_TYPE_M2P,
      .burst = DMA_BURST_1,
      .width = DMA_WIDTH_BYTE
  };

  interface->rxDma = init(GpDmaList, &rxChannelConfig);
  if (!interface->rxDma)
    return E_ERROR;
  dmaCallback(interface->rxDma, rxDmaHandler, interface);

  interface->txDma = init(GpDma, &txChannelConfig);
  if (!interface->txDma)
    return E_ERROR;
  dmaCallback(interface->txDma, txDmaHandler, interface);

  return E_OK;
}
/*----------------------------------------------------------------------------*/
#ifdef CONFIG_SERIAL_PM
static enum result powerStateHandler(void *object, enum pmState state)
{
  struct SerialDma * const interface = object;
  struct UartRateConfig rateConfig;
  enum result res;

  if (state == PM_ACTIVE)
  {
    /* Recalculate and set baud rate */
    if ((res = uartCalcRate(object, interface->rate, &rateConfig)) != E_OK)
      return res;

    uartSetRate(object, rateConfig);
  }

  return E_OK;
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

  /* Allocate one buffer for transmission and three buffers for reception */
  interface->pool = malloc(SINGLE_BUFFER_SIZE * (1 + RECEPTION_BUFFERS));
  interface->txBuffer = interface->pool;
  interface->rxBuffer = interface->pool + SINGLE_BUFFER_SIZE;

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

#ifdef CONFIG_SERIAL_PM
  if ((res = pmRegister(interface, powerStateHandler)) != E_OK)
    return res;
#endif

  /* Start reception */
  dmaStart(interface->rxDma, interface->rxBuffer, (const void *)&reg->RBR,
      SINGLE_BUFFER_SIZE * RECEPTION_BUFFERS);

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static void serialDeinit(void *object)
{
  struct SerialDma * const interface = object;

  /* Stop channels */
  dmaStop(interface->rxDma);

#ifdef CONFIG_SERIAL_PM
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
      *(uint32_t *)data = byteQueueSize(&interface->rxQueue);
      return E_OK;

    case IF_PENDING:
      *(uint32_t *)data = byteQueueSize(&interface->txQueue);
      return E_OK;

    default:
      return E_ERROR;
  }
}
/*----------------------------------------------------------------------------*/
static enum result serialSet(void *object, enum ifOption option,
    const void *data)
{
  struct SerialDma * const interface = object;
  struct UartRateConfig rateConfig;
  enum result res;

  switch (option)
  {
    case IF_RATE:
      res = uartCalcRate(object, *(const uint32_t *)data, &rateConfig);

      if (res == E_OK)
      {
        interface->rate = *(const uint32_t *)data;
        uartSetRate(object, rateConfig);
      }
      return res;

    default:
      return E_ERROR;
  }
}
/*----------------------------------------------------------------------------*/
static uint32_t serialRead(void *object, uint8_t *buffer, uint32_t length)
{
  struct SerialDma * const interface = object;
  uint32_t read;
  irqState state;

  state = irqSave();
  read = byteQueuePopArray(&interface->rxQueue, buffer, length);
  irqRestore(state);

  return read;
}
/*----------------------------------------------------------------------------*/
static uint32_t serialWrite(void *object, const uint8_t *buffer,
    uint32_t length)
{
  struct SerialDma * const interface = object;
  LPC_UART_Type * const reg = interface->base.reg;
  uint32_t chunkLength = 0;
  uint32_t sourceLength = length;

  /*
   * Disable interrupts before status check because DMA interrupt
   * may be called and transmission will stall.
   */
  const irqState state = irqSave();

  if (dmaStatus(interface->txDma) != E_BUSY)
  {
    chunkLength = length < SINGLE_BUFFER_SIZE ? length : SINGLE_BUFFER_SIZE;

    memcpy(interface->txBuffer, buffer, chunkLength);

    length -= chunkLength;
    buffer += chunkLength;
  }
  length -= byteQueuePushArray(&interface->txQueue, buffer, length);

  irqRestore(state);

  if (chunkLength)
  {
    const enum result res = dmaStart(interface->txDma, (void *)&reg->THR,
        interface->txBuffer, chunkLength);

    if (res != E_OK)
      return 0;
  }

  return sourceLength - length;
}
