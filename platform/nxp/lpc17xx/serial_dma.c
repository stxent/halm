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
static enum result dmaSetup(struct SerialDma *, int8_t, int8_t);
static void interruptHandler(void *);
/*----------------------------------------------------------------------------*/
static enum result serialInit(void *, const void *);
static void serialDeinit(void *);
static enum result serialCallback(void *, void (*)(void *), void *);
static enum result serialGet(void *, enum ifOption, void *);
static enum result serialSet(void *, enum ifOption, const void *);
static uint32_t serialRead(void *, uint8_t *, uint32_t);
static uint32_t serialWrite(void *, const uint8_t *, uint32_t);
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
static enum result dmaSetup(struct SerialDma *interface, int8_t rxChannel,
    int8_t txChannel)
{
  struct GpdmaConfig channels[2] = {
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

  interface->rxDma = init(Gpdma, channels + 0);
  if (!interface->rxDma)
    return E_ERROR;
  interface->txDma = init(Gpdma, channels + 1);
  if (!interface->txDma)
  {
    deinit(interface->rxDma);
    return E_ERROR;
  }
  return E_OK;
}
/*----------------------------------------------------------------------------*/
static void interruptHandler(void *object __attribute__((unused)))
{

}
/*----------------------------------------------------------------------------*/
static enum result serialInit(void *object, const void *configPtr)
{
  /* Set pointer to interface configuration data */
  const struct SerialDmaConfig * const config = configPtr;
  struct SerialDma *interface = object;
  struct UartConfig parentConfig;
  enum result res;

  /* Check interface configuration data */
  assert(config);

  /* Initialize parent configuration structure */
  parentConfig.channel = config->channel;
  parentConfig.rx = config->rx;
  parentConfig.tx = config->tx;
  parentConfig.rate = config->rate;
  parentConfig.parity = config->parity;

  /* Call UART class constructor */
  if ((res = Uart->init(object, &parentConfig)) != E_OK)
    return res;

  /* Set pointer to hardware interrupt handler */
  interface->parent.handler = interruptHandler;

  if ((res = dmaSetup(interface, config->rxChannel, config->txChannel)) != E_OK)
  {
    Uart->deinit(interface);
    return res;
  }

  /* Enable and clear FIFO, set RX trigger level to 8 bytes */
  interface->parent.reg->FCR &= ~FCR_RX_TRIGGER_MASK;
  interface->parent.reg->FCR |= FCR_ENABLE | FCR_DMA_ENABLE
      | FCR_RX_TRIGGER(RX_FIFO_LEVEL);
  interface->parent.reg->TER = TER_TXEN;
  /* All interrupts are disabled by default */
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
  Uart->deinit(interface);
}
/*----------------------------------------------------------------------------*/
static enum result serialCallback(void *object, void (*callback)(void *),
    void *argument)
{
  /* Callback functionality not implemented */
  return E_ERROR;
}
/*----------------------------------------------------------------------------*/
static enum result serialGet(void *object, enum ifOption option, void *data)
{
  struct SerialDma *interface = object;
  enum result res;

  switch (option)
  {
    case IF_RATE:
      /* TODO */
      return E_ERROR;
    default:
      return E_ERROR;
  }
}
/*----------------------------------------------------------------------------*/
static enum result serialSet(void *object, enum ifOption option,
    const void *data)
{
  struct SerialDma *interface = object;
  enum result res;

  switch (option)
  {
    case IF_RATE:
    {
      struct UartRateConfig rate;

      if ((res = uartCalcRate(&rate, *(uint32_t *)data)) != E_OK)
        return res;
      uartSetRate(object, rate);
      return E_OK;
    }
    default:
      return E_ERROR;
  }
}
/*----------------------------------------------------------------------------*/
static uint32_t serialRead(void *object, uint8_t *buffer, uint32_t length)
{
  struct SerialDma *interface = object;
  const void *source =
      (const void *)&((LPC_UART_TypeDef *)interface->parent.reg)->RBR;
  uint32_t read = 0;

  mutexLock(&interface->dmaLock);
  /* TODO Add DMA error handling */
  if (length && dmaStart(interface->rxDma,
      buffer, (void *)&interface->parent.reg->RBR, length) == E_OK)
  {
    while (dmaIsActive(interface->rxDma));
    read = length;
  }

  return read;
}
/*----------------------------------------------------------------------------*/
static uint32_t serialWrite(void *object, const uint8_t *buffer,
    uint32_t length)
{
  struct SerialDma *interface = object;
  uint32_t written = 0;

  /* TODO Add DMA error handling */
  if (length && dmaStart(interface->txDma,
      (void *)&interface->parent.reg->THR, buffer, length) == E_OK)
  {
    /* Wait until all bytes transferred */
    while (dmaIsActive(interface->txDma));
    written = length;
  }

  return written;
}
