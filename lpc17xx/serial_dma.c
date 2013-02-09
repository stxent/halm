/*
 * serial.c
 * Copyright (C) 2012 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <string.h>
/*----------------------------------------------------------------------------*/
#include "gpdma.h"
#include "serial_dma.h"
#include "uart_defs.h"
/*----------------------------------------------------------------------------*/
/* Serial port settings */
//#define TX_FIFO_SIZE                    8
//#define RX_FIFO_LEVEL                   2 /* 8 characters */
//#define DEFAULT_PRIORITY                15 /* Lowest priority in Cortex-M3 */
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
      Uart->parent.deinit(device); //FIXME
      break;
    default:
      break;
  }
}
/*----------------------------------------------------------------------------*/
static void serialHandler(void *object /*__attribute__((unused))*/)
{

}
/*----------------------------------------------------------------------------*/
static enum result serialGetOpt(void *object, enum ifOption option, void *data)
{
  struct SerialDma *device = object;

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
static enum result serialSetOpt(void *object, enum ifOption option,
    const void *data)
{
  struct SerialDma *device = object;

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
static enum result serialDmaSetup(struct SerialDma *device, int8_t rxChannel,
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
static void serialDeinit(void *object)
{
  struct SerialDma *device = object;

  /* Release resources */
  serialCleanup(device, FREE_ALL);
}
/*----------------------------------------------------------------------------*/
static enum result serialInit(void *object, const void *configPtr)
{
  /* Set pointer to device configuration data */
  const struct SerialDmaConfig *config = configPtr;
  enum result res;
  struct SerialDma *device = object;

  /* Call UART class constructor */
  if ((res = Uart->parent.init(object, configPtr)) != E_OK)
    return res;

  if ((res = serialDmaSetup(device,
      config->rxChannel, config->txChannel)) != E_OK)
  {
    serialCleanup(device, FREE_PERIPHERAL);
    return res;
  }

  if ((res = uartSetRate(object, uartCalcRate(config->rate))) != E_OK)
  {
    serialCleanup(device, FREE_ALL);
    return res;
  }

  /* Enable and clear FIFO, set RX trigger level to 8 bytes */
  device->parent.reg->FCR &= ~FCR_RX_TRIGGER_MASK;
  device->parent.reg->FCR |= FCR_ENABLE | FCR_DMA_ENABLE | FCR_RX_TRIGGER(0);
  device->parent.reg->TER = TER_TXEN;

//  /* Enable RBR and THRE interrupts */
//  device->parent.reg->IER = IER_RBR | IER_THRE;

  /* Set interrupt priority, lowest by default */
//  NVIC_SetPriority(device->irq, DEFAULT_PRIORITY);
  /* Enable UART interrupt */
//  NVIC_EnableIRQ(device->parent.irq);
  return E_OK;
}
