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
//  FREE_RX_QUEUE,
//  FREE_TX_QUEUE,
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
//      //FIXME
//      free(device->rxBuffer.data);
//      free(device->txBuffer.data);
//      NVIC_DisableIRQ(device->parent.irq); /* Disable interrupt */
//    case FREE_TX_QUEUE:
//      queueDeinit(&device->txQueue);
//    case FREE_RX_QUEUE:
//      queueDeinit(&device->rxQueue);
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
//  struct Serial *device = (struct Serial *)base; //FIXME
//  uint8_t counter = 0;
//
//  /* Interrupt status cleared when performed read operation on IIR register */
//  switch (device->parent.reg->IIR & IIR_INT_MASK)
//  {
//    case IIR_INT_RDA:
//    case IIR_INT_CTI:
//      /* Byte removed from FIFO after RBR register read operation */
//      while (device->parent.reg->LSR & LSR_RDR)
//        queuePush(&device->rxQueue, device->parent.reg->RBR);
//      break;
//    case IIR_INT_THRE:
//      /* Fill FIFO with 8 bytes or less */
//      while (queueSize(&device->txQueue) && counter++ < TX_FIFO_SIZE)
//        device->parent.reg->THR = queuePop(&device->txQueue);
//      break;
//    default:
//      break;
//  }
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
//    case IF_QUEUE_RX:
//      *(uint16_t *)data = device->rxBuffer.capacity;
//      return E_OK;
//    case IF_QUEUE_TX:
//      *(uint8_t *)data = queueSize(&device->txQueue);
//      return E_OK;
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
//  uint32_t read = 0;

  if (length && dmaStart(device->rxDma,
      buffer, (void *)&device->parent.reg->RBR, length) == E_OK)
  {
    while (dmaIsActive(device->rxDma));
    return length;
  }
  return 0;
//  if (dmaIsActive(device->rxDma))
//  {
//    dmaHalt(device->rxDma);
//    while (dmaIsActive(device->rxDma));
//    device->rxBuffer.size += dmaGetCount(device->rxDma);
//  }
//  read = length > device->rxBuffer.size ? device->rxBuffer.size : length;
//  memcpy(buffer, device->rxBuffer.data, read);
//  device->rxBuffer.size -= read;
//  if (device->rxBuffer.size)
//  {
//    memmove(device->rxBuffer.data, device->rxBuffer.data + read,
//        device->rxBuffer.size);
//  }
//  if (device->rxBuffer.capacity > device->rxBuffer.size)
//  {
//    if (dmaStart(device->rxDma, (void *)device->rxBuffer.data,
//        (void *)&device->parent.reg->RBR, device->rxBuffer.capacity -
//        device->rxBuffer.size) == E_OK)
//    {
//      /* TODO Add error handling */
//    }
//  }
//  return read;
}
/*----------------------------------------------------------------------------*/
static uint32_t serialWrite(struct Interface *interface, const uint8_t *buffer,
    uint32_t length)
{
  struct SerialDma *device = (struct SerialDma *)interface;
//  uint32_t written;
//
//  written = length > device->txBuffer.size ? device->txBuffer.size : length;

  if (length && dmaStart(device->txDma,
      (void *)&device->parent.reg->THR, buffer, length) == E_OK)
  {
    /* Wait until all bytes transferred */
    while (dmaIsActive(device->txDma));
    return length;
  }
  /* TODO Add DMA error handling */
  return 0;
}
/*----------------------------------------------------------------------------*/
static enum result serialDmaSetup(struct SerialDma *device, int8_t rxChannel,
    int8_t txChannel)
{
  struct DmaConfig dmaTxConfig = {
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
  };
  struct DmaConfig dmaRxConfig = {
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
  };

  device->rxDma = init(Dma, &dmaRxConfig);
  if (!device->rxDma)
    return E_ERROR;
  device->txDma = init(Dma, &dmaTxConfig);
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

//  //FIXME
//  device->rxBuffer.data = (uint8_t *)malloc(config->rxLength);
//  device->txBuffer.data = (uint8_t *)malloc(config->txLength);

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

  /* Enable UART interrupt and set priority, lowest by default */
//  NVIC_EnableIRQ(device->parent.irq);
  return E_OK;
}
