/*
 * serial.c
 * Copyright (C) 2012 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <assert.h>
#include "platform/nxp/uart_defs.h"
#include "platform/nxp/lpc17xx/gpdma.h"
#include "platform/nxp/lpc17xx/serial_dma.h"
/*----------------------------------------------------------------------------*/
#define RX_FIFO_LEVEL 0 /* 1 character */
/*----------------------------------------------------------------------------*/
static void dmaHandler(void *);
static enum result dmaSetup(struct SerialDma *, int8_t, int8_t);
/*----------------------------------------------------------------------------*/
static enum result serialInit(void *, const void *);
static void serialDeinit(void *);
static enum result serialCallback(void *, void (*)(void *), void *);
static enum result serialGet(void *, enum ifOption, void *);
static enum result serialSet(void *, enum ifOption, const void *);
static uint32_t serialRead(void *, uint8_t *, uint32_t);
static uint32_t serialWrite(void *, const uint8_t *, uint32_t);
/*----------------------------------------------------------------------------*/
static const enum gpDmaLine dmaTxLines[] = {
    GPDMA_LINE_UART0_TX,
    GPDMA_LINE_UART1_TX,
    GPDMA_LINE_UART2_TX,
    GPDMA_LINE_UART3_TX
};
/*----------------------------------------------------------------------------*/
static const enum gpDmaLine dmaRxLines[] = {
    GPDMA_LINE_UART0_RX,
    GPDMA_LINE_UART1_RX,
    GPDMA_LINE_UART2_RX,
    GPDMA_LINE_UART3_RX
};
/*----------------------------------------------------------------------------*/
/* We like structures so we put a structure in a structure */
/* So we can initialize a structure while we initialize a structure */
static const struct InterfaceClass serialDmaTable = {
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
const struct InterfaceClass *SerialDma = &serialDmaTable;
/*----------------------------------------------------------------------------*/
static void dmaHandler(void *object)
{
  struct SerialDma *interface = object;

  if (interface->callback)
    interface->callback(interface->callbackArgument);
}
/*----------------------------------------------------------------------------*/
static enum result dmaSetup(struct SerialDma *interface, int8_t rxChannel,
    int8_t txChannel)
{
  struct GpDmaConfig channels[2] = {
      {
          .channel = rxChannel,
          .source = {
              .line = dmaRxLines[interface->parent.channel],
              .increment = false
          },
          .destination = {
              .line = GPDMA_LINE_MEMORY,
              .increment = true
          },
          .direction = GPDMA_DIR_P2M,
          .burst = DMA_BURST_1,
          .width = DMA_WIDTH_BYTE
      }, {
          .channel = txChannel,
          .source = {
              .line = GPDMA_LINE_MEMORY,
              .increment = true
          },
          .destination = {
              .line = dmaTxLines[interface->parent.channel],
              .increment = false
          },
          .direction = GPDMA_DIR_M2P,
          .burst = DMA_BURST_1,
          .width = DMA_WIDTH_BYTE
      }
  };

  interface->rxDma = init(GpDma, channels + 0);
  if (!interface->rxDma)
    return E_ERROR;
  dmaCallback(interface->rxDma, dmaHandler, interface);
  interface->txDma = init(GpDma, channels + 1);
  if (!interface->txDma)
    return E_ERROR;
  dmaCallback(interface->txDma, dmaHandler, interface);
  return E_OK;
}
/*----------------------------------------------------------------------------*/
static enum result serialInit(void *object, const void *configPtr)
{
  /* Set pointer to interface configuration data */
  const struct SerialDmaConfig * const config = configPtr;
  struct SerialDma *interface = object;
  struct UartBaseConfig parentConfig = {
      .channel = config->channel,
      .rx = config->rx,
      .tx = config->tx
  };
  enum result res;

  /* Call UART class constructor */
  if ((res = UartBase->init(object, &parentConfig)) != E_OK)
    return res;

  if ((res = dmaSetup(interface, config->rxChannel, config->txChannel)) != E_OK)
    return res;

  /* Initialize UART block */
  LPC_UART_Type *reg = interface->parent.reg;

  /* Set 8-bit length */
  reg->LCR = LCR_WORD_8BIT;
  /* Enable FIFO and DMA, set RX trigger level */
  reg->FCR = (reg->FCR & ~FCR_RX_TRIGGER_MASK) | FCR_RX_TRIGGER(RX_FIFO_LEVEL)
      | FCR_ENABLE | FCR_DMA_ENABLE;
  /* Disable all interrupts */
  reg->IER = 0;
  /* Enable transmitter */
  reg->TER = TER_TXEN;

  uartSetParity(object, config->parity);
  uartSetRate(object, uartCalcRate(object, config->rate));

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static void serialDeinit(void *object)
{
  struct SerialDma *interface = object;

  /* Free DMA channel descriptors */
  deinit(interface->txDma);
  deinit(interface->rxDma);
  /* Call UART class destructor */
  UartBase->deinit(interface);
}
/*----------------------------------------------------------------------------*/
static enum result serialCallback(void *object, void (*callback)(void *),
    void *argument)
{
  struct SerialDma *interface = object;

  interface->callback = callback;
  interface->callbackArgument = argument;
  return E_OK;
}
/*----------------------------------------------------------------------------*/
static enum result serialGet(void *object, enum ifOption option,
    void *data __attribute__((unused)))
{
  /* TODO Add more options to SerialDma */
  struct SerialDma *interface = object;

  switch (option)
  {
    case IF_READY:
      return dmaActive(interface->rxDma) || dmaActive(interface->txDma) ?
          E_BUSY : E_OK;
    default:
      return E_ERROR;
  }
}
/*----------------------------------------------------------------------------*/
static enum result serialSet(void *object, enum ifOption option,
    const void *data)
{
  struct SerialDma *interface = object;

  switch (option)
  {
    case IF_BLOCKING:
      interface->blocking = true;
      return E_OK;
    case IF_RATE:
      uartSetRate(object, uartCalcRate(object, *(uint32_t *)data));
      return E_OK;
    case IF_ZEROCOPY:
      interface->blocking = false;
      return E_OK;
    default:
      return E_ERROR;
  }
}
/*----------------------------------------------------------------------------*/
static uint32_t serialRead(void *object, uint8_t *buffer, uint32_t length)
{
  struct SerialDma *interface = object;
  const void *source =
      (const void *)&((LPC_UART_Type *)interface->parent.reg)->RBR;
  uint32_t read = 0;

  /* TODO Add DMA error handling in SerialDma */
  if (length && dmaStart(interface->rxDma, buffer, source, length) == E_OK)
  {
    if (interface->blocking)
      while (dmaActive(interface->rxDma));
    read = length;
  }

  return read;
}
/*----------------------------------------------------------------------------*/
static uint32_t serialWrite(void *object, const uint8_t *buffer,
    uint32_t length)
{
  struct SerialDma *interface = object;
  void *destination = (void *)&((LPC_UART_Type *)interface->parent.reg)->THR;
  uint32_t written = 0;

  if (length && dmaStart(interface->txDma, destination, buffer, length) == E_OK)
  {
    if (interface->blocking)
      while (dmaActive(interface->txDma));
    written = length;
  }

  return written;
}
