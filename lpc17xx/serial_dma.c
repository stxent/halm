/*
 * serial.c
 * Copyright (C) 2012 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <assert.h>
#include <string.h>
#include "gpdma.h"
#include "serial_dma.h"
#include "uart_defs.h"
/*----------------------------------------------------------------------------*/
/* Serial port settings */
#define RX_FIFO_LEVEL                   0 /* 1 character */
/*----------------------------------------------------------------------------*/
static enum result dmaSetup(struct SerialDma *, int8_t, int8_t);
/*----------------------------------------------------------------------------*/
static void serialHandler(void *);
static enum result serialInit(void *, const void *);
static void serialDeinit(void *);
static uint32_t serialRead(void *, uint8_t *, uint32_t);
static uint32_t serialWrite(void *, const uint8_t *, uint32_t);
static enum result serialGetOpt(void *, enum ifOption, void *);
static enum result serialSetOpt(void *, enum ifOption,
    const void *);
/*----------------------------------------------------------------------------*/
static const enum gpdmaLine dmaTxLines[] = {
    GPDMA_LINE_UART0_TX,
    GPDMA_LINE_UART1_TX,
    GPDMA_LINE_UART2_TX,
    GPDMA_LINE_UART3_TX
};
/*----------------------------------------------------------------------------*/
static const enum gpdmaLine dmaRxLines[] = {
    GPDMA_LINE_UART0_RX,
    GPDMA_LINE_UART1_RX,
    GPDMA_LINE_UART2_RX,
    GPDMA_LINE_UART3_RX
};
/*----------------------------------------------------------------------------*/
/* We like structures so we put a structure in a structure */
/* So we can initialize a structure while we initialize a structure */
static const struct UartClass serialDmaTable = {
    .parent = {
        .size = sizeof(struct SerialDma),
        .init = serialInit,
        .deinit = serialDeinit,

        .read = serialRead,
        .write = serialWrite,
        .getopt = serialGetOpt,
        .setopt = serialSetOpt
    },
    .handler = 0
};
/*----------------------------------------------------------------------------*/
const struct UartClass *SerialDma = &serialDmaTable;
/*----------------------------------------------------------------------------*/
static enum result dmaSetup(struct SerialDma *device, int8_t rxChannel,
    int8_t txChannel)
{
  struct GpdmaConfig channels[2] =
  {
      {
          .channel = rxChannel,
          .source = {
              .line = dmaRxLines[device->parent.channel],
              .increment = false
          },
          .destination = {
              .line = GPDMA_LINE_MEMORY,
              .increment = true
          },
          .direction = GPDMA_DIR_P2M,
          .burst = DMA_BURST_1,
          .width = DMA_WIDTH_BYTE
      },
      {
          .channel = txChannel,
          .source = {
              .line = GPDMA_LINE_MEMORY,
              .increment = true
          },
          .destination = {
              .line = dmaTxLines[device->parent.channel],
              .increment = false
          },
          .direction = GPDMA_DIR_M2P,
          .burst = DMA_BURST_1,
          .width = DMA_WIDTH_BYTE
      }
  };

  device->rxDma = init(Gpdma, channels + 0);
  if (!device->rxDma)
    return E_ERROR;
  device->txDma = init(Gpdma, channels + 1);
  if (!device->txDma)
  {
    deinit(device->rxDma);
    return E_ERROR;
  }
  return E_OK;
}
/*----------------------------------------------------------------------------*/
static enum result serialInit(void *object, const void *configPtr)
{
  /* Set pointer to device configuration data */
  const struct SerialDmaConfig *config = configPtr;
  const struct UartConfig parentConfig = {
      .channel = config->channel,
      .rx = config->rx,
      .tx = config->tx,
      .rate = config->rate,
      .parity = config->parity
  };
  enum result res;
  struct SerialDma *device = object;

  /* Call UART class constructor */
  if ((res = Uart->parent.init(object, &parentConfig)) != E_OK)
    return res;

  if ((res = dmaSetup(device, config->rxChannel, config->txChannel)) != E_OK)
  {
    Uart->parent.deinit(device);
    return res;
  }

  /* Enable and clear FIFO, set RX trigger level to 8 bytes */
  device->parent.reg->FCR &= ~FCR_RX_TRIGGER_MASK;
  device->parent.reg->FCR |= FCR_ENABLE | FCR_DMA_ENABLE
      | FCR_RX_TRIGGER(RX_FIFO_LEVEL);
  device->parent.reg->TER = TER_TXEN;
  /* All interrupts are disabled by default */
  return E_OK;
}
/*----------------------------------------------------------------------------*/
static void serialDeinit(void *object)
{
  struct SerialDma *device = object;

  /* Free DMA channel descriptors */
  deinit(device->txDma);
  deinit(device->rxDma);
  /* Call UART class destructor */
  Uart->parent.deinit(device);
}
/*----------------------------------------------------------------------------*/
static uint32_t serialRead(void *object, uint8_t *buffer, uint32_t length)
{
  struct SerialDma *device = object;

  /* TODO Add DMA error handling */
  if (length && dmaStart(device->rxDma,
      buffer, (void *)&device->parent.reg->RBR, length) == E_OK)
  {
    while (dmaIsActive(device->rxDma));
    return length;
  }
  return 0;
}
/*----------------------------------------------------------------------------*/
static uint32_t serialWrite(void *object, const uint8_t *buffer,
    uint32_t length)
{
  struct SerialDma *device = object;

  /* TODO Add DMA error handling */
  if (length && dmaStart(device->txDma,
      (void *)&device->parent.reg->THR, buffer, length) == E_OK)
  {
    /* Wait until all bytes transferred */
    while (dmaIsActive(device->txDma));
    return length;
  }
  return 0;
}
/*----------------------------------------------------------------------------*/
static enum result serialGetOpt(void *object, enum ifOption option, void *data)
{
  return E_ERROR;
}
/*----------------------------------------------------------------------------*/
static enum result serialSetOpt(void *object, enum ifOption option,
    const void *data)
{
  struct SerialDma *device = object;

  switch (option)
  {
    case IF_SPEED:
      uartSetRate((struct Uart *)device, uartCalcRate(*(uint32_t *)data));
      return E_OK; /* TODO */
    default:
      return E_ERROR;
  }
}
