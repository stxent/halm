/*
 * serial.c
 * Copyright (C) 2012 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <string.h>
/*----------------------------------------------------------------------------*/
#include "uart_defs.h"
#include "serial_dma.h"
/*----------------------------------------------------------------------------*/
///* Serial port settings */
//#define TX_FIFO_SIZE                    8
//#define RX_FIFO_LEVEL                   2 /* 8 characters */
/*----------------------------------------------------------------------------*/
enum cleanup
{
  FREE_NONE = 0,
  FREE_PERIPHERAL,
  FREE_DMA,
  FREE_ALL
};
/*----------------------------------------------------------------------------*/
static void serialCleanup(struct SerialDma *, enum cleanup);
static enum result serialDmaSetup(struct SerialDma *, int8_t, int8_t);
/*----------------------------------------------------------------------------*/
static void serialHandler(struct Uart *);
static enum result serialInit(struct Interface *, const void *);
static void serialDeinit(struct Interface *);
static uint32_t serialRead(struct Interface *, uint8_t *, uint32_t);
static uint32_t serialWrite(struct Interface *, const uint8_t *, uint32_t);
static enum result serialGetOpt(struct Interface *, enum ifOption, void *);
static enum result serialSetOpt(struct Interface *, enum ifOption,
    const void *);
/*----------------------------------------------------------------------------*/
static const enum dmaLine dmaTxLines[] = {
    DMA_LINE_UART0_TX,
    DMA_LINE_UART1_TX,
    DMA_LINE_UART2_TX,
    DMA_LINE_UART3_TX
};
/*----------------------------------------------------------------------------*/
static const enum dmaLine dmaRxLines[] = {
    DMA_LINE_UART0_RX,
    DMA_LINE_UART1_RX,
    DMA_LINE_UART2_RX,
    DMA_LINE_UART3_RX
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
    .handler = serialHandler
};
/*----------------------------------------------------------------------------*/
const struct UartClass *SerialDma = &serialDmaTable;
/*----------------------------------------------------------------------------*/
static void serialCleanup(struct SerialDma *device, enum cleanup step)
{
  switch (step)
  {
    case FREE_ALL:
    case FREE_DMA:
      /* Free DMA channel descriptors */
      deinit(device->txDma);
      deinit(device->rxDma);
    case FREE_PERIPHERAL:
      /* Call UART class destructor */
      Uart->parent.deinit((struct Interface *)device); //FIXME
      break;
    default:
      break;
  }
}
/*----------------------------------------------------------------------------*/
static void serialHandler(struct Uart *base /*__attribute__((unused))*/)
{

}
/*----------------------------------------------------------------------------*/
static enum result serialGetOpt(struct Interface *interface,
    enum ifOption option, void *data)
{
  struct SerialDma *device = (struct SerialDma *)interface;

  switch (option)
  {
    case IF_SPEED:
      /* TODO */
      return E_OK;
    default:
      return E_ERROR;
  }
}
/*----------------------------------------------------------------------------*/
static enum result serialSetOpt(struct Interface *interface,
    enum ifOption option, const void *data)
{
  struct SerialDma *device = (struct SerialDma *)interface;

  switch (option)
  {
    case IF_SPEED:
      return uartSetRate((struct Uart *)device,
          uartCalcRate(*(uint32_t *)data));
    default:
      return E_ERROR;
  }
}
/*----------------------------------------------------------------------------*/
static uint32_t serialRead(struct Interface *interface, uint8_t *buffer,
    uint32_t length)
{
  struct SerialDma *device = (struct SerialDma *)interface;

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
static uint32_t serialWrite(struct Interface *interface, const uint8_t *buffer,
    uint32_t length)
{
  struct SerialDma *device = (struct SerialDma *)interface;

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
static enum result serialDmaSetup(struct SerialDma *device, int8_t rxChannel,
    int8_t txChannel)
{
  struct DmaConfig channels[2] =
  {
      {
          .channel = rxChannel,
          .source = {
              .line = dmaRxLines[device->parent.channel],
              .increment = false
          },
          .destination = {
              .line = DMA_LINE_MEMORY,
              .increment = true
          },
          .direction = DMA_DIR_P2M,
          .burst = DMA_BURST_1,
          .width = DMA_WIDTH_BYTE
      },
      {
          .channel = txChannel,
          .source = {
              .line = DMA_LINE_MEMORY,
              .increment = true
          },
          .destination = {
              .line = dmaTxLines[device->parent.channel],
              .increment = false
          },
          .direction = DMA_DIR_M2P,
          .burst = DMA_BURST_1,
          .width = DMA_WIDTH_BYTE
      }
  };

  device->rxDma = init(Dma, channels + 0);
  if (!device->rxDma)
    return E_ERROR;
  device->txDma = init(Dma, channels + 1);
  if (!device->txDma)
  {
    deinit(device->rxDma);
    return E_ERROR;
  }
  return E_OK;
}
/*----------------------------------------------------------------------------*/
static void serialDeinit(struct Interface *interface)
{
  struct SerialDma *device = (struct SerialDma *)interface;

  /* Release resources */
  serialCleanup(device, FREE_ALL);
}
/*----------------------------------------------------------------------------*/
static enum result serialInit(struct Interface *interface,
    const void *configPtr)
{
  /* Set pointer to device configuration data */
  const struct SerialDmaConfig *config =
      (const struct SerialDmaConfig *)configPtr;
  struct SerialDma *device = (struct SerialDma *)interface;

  /* Call UART class constructor */
  if (Uart->parent.init(interface, configPtr) != E_OK)
    return E_ERROR;

  if (serialDmaSetup(device, config->rxChannel, config->txChannel) != E_OK)
  {
    serialCleanup(device, FREE_PERIPHERAL);
    return E_ERROR;
  }

  if (uartSetRate((struct Uart *)device, uartCalcRate(config->rate)) != E_OK)
  {
    serialCleanup(device, FREE_ALL);
    return E_ERROR;
  }

  /* Set 8-bit length */
  device->parent.reg->LCR = LCR_WORD_8BIT;
  /* Enable and clear FIFO, enable DMA, set RX trigger level to 1 byte */
  device->parent.reg->FCR = FCR_ENABLE | FCR_DMA_ENABLE | FCR_RX_TRIGGER(0);
  /* Enable RBR and THRE interrupts */
//  device->parent.reg->IER = IER_RBR | IER_THRE;
  device->parent.reg->TER = TER_TXEN;

  /* Enable UART interrupt */
//  NVIC_EnableIRQ(device->parent.irq);
  return E_OK;
}
