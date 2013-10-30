/*
 * spi_dma.c
 * Copyright (C) 2013 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <platform/nxp/ssp_defs.h>
#include <platform/nxp/lpc17xx/gpdma.h>
#include <platform/nxp/lpc17xx/spi_dma.h>
/*----------------------------------------------------------------------------*/
static void dmaHandler(void *);
static enum result dmaSetup(struct SpiDma *, int8_t, int8_t);
static void interruptHandler(void *);
/*----------------------------------------------------------------------------*/
static enum result spiInit(void *, const void *);
static void spiDeinit(void *);
static enum result spiCallback(void *, void (*)(void *), void *);
static enum result spiGet(void *, enum ifOption, void *);
static enum result spiSet(void *, enum ifOption, const void *);
static uint32_t spiRead(void *, uint8_t *, uint32_t);
static uint32_t spiWrite(void *, const uint8_t *, uint32_t);
/*----------------------------------------------------------------------------*/
static const enum gpDmaLine dmaTxLines[] = {
    GPDMA_LINE_SSP0_TX,
    GPDMA_LINE_SSP1_TX
};

static const enum gpDmaLine dmaRxLines[] = {
    GPDMA_LINE_SSP0_RX,
    GPDMA_LINE_SSP1_RX
};
/*----------------------------------------------------------------------------*/
static const struct InterfaceClass spiTable = {
    .size = sizeof(struct SpiDma),
    .init = spiInit,
    .deinit = spiDeinit,

    .callback = spiCallback,
    .get = spiGet,
    .set = spiSet,
    .read = spiRead,
    .write = spiWrite
};
/*----------------------------------------------------------------------------*/
const struct InterfaceClass *SpiDma = &spiTable;
/*----------------------------------------------------------------------------*/
static void dmaHandler(void *object)
{
  struct SpiDma *interface = object;

  if (interface->callback)
    interface->callback(interface->callbackArgument);
}
/*----------------------------------------------------------------------------*/
static enum result dmaSetup(struct SpiDma *interface, int8_t rxChannel,
    int8_t txChannel)
{
  struct GpDmaConfig channels[3] = {
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
      }, {
          .channel = txChannel,
          .source = {
              .line = GPDMA_LINE_MEMORY,
              .increment = false
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
  interface->txDma = init(GpDma, channels + 1);
  if (!interface->txDma)
    return E_ERROR;
  interface->txMockDma = init(GpDma, channels + 2);
  if (!interface->txMockDma)
    return E_ERROR;
  return E_OK;
}
/*----------------------------------------------------------------------------*/
static void interruptHandler(void *object)
{
  struct SpiDma *interface = object;
  LPC_SSP_Type *reg = interface->parent.reg;

  reg->IMSC &= ~IMSC_RTIM;
  if (interface->callback)
    interface->callback(interface->callbackArgument);
}
/*----------------------------------------------------------------------------*/
static enum result spiInit(void *object, const void *configPtr)
{
  const struct SpiDmaConfig * const config = configPtr;
  const struct SspBaseConfig parentConfig = {
      .channel = config->channel,
      .miso = config->miso,
      .mosi = config->mosi,
      .sck = config->sck
  };
  struct SpiDma *interface = object;
  enum result res;

  /* Call SSP class constructor */
  if ((res = SspBase->init(object, &parentConfig)) != E_OK)
    return res;

  if ((res = dmaSetup(interface, config->rxChannel, config->txChannel)) != E_OK)
    return res;

  interface->blocking = true;
  interface->callback = 0;
  interface->lock = SPIN_UNLOCKED;

  /* Set pointer to interrupt handler */
  interface->parent.handler = interruptHandler;

  /* Initialize SSP block */
  LPC_SSP_Type *reg = interface->parent.reg;

  /* Set frame size */
  reg->CR0 = CR0_DSS(8);

  /* Set mode for SPI interface */
  if (config->mode & 0x01)
    reg->CR0 |= CR0_CPHA;
  if (config->mode & 0x02)
    reg->CR0 |= CR0_CPOL;

  sspSetRate(object, config->rate);
  /* Enable peripheral */
  reg->CR1 = CR1_SSE;

  /* Set lowest interrupt priority */
  irqSetPriority(interface->parent.irq, 0);
  /* Enable SPI interrupt */
  irqEnable(interface->parent.irq);

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static void spiDeinit(void *object)
{
  struct SpiDma *interface = object;

  /* Free DMA channel descriptors */
  deinit(interface->txMockDma);
  deinit(interface->txDma);
  deinit(interface->rxDma);

  /* Call SSP class destructor */
  SspBase->deinit(interface);
}
/*----------------------------------------------------------------------------*/
static enum result spiCallback(void *object, void (*callback)(void *),
    void *argument)
{
  struct SpiDma *interface = object;

  interface->callback = callback;
  interface->callbackArgument = argument;
  return E_OK;
}
/*----------------------------------------------------------------------------*/
static enum result spiGet(void *object, enum ifOption option, void *data)
{
  struct SpiDma *interface = object;
  LPC_SSP_Type *reg = interface->parent.reg;

  switch (option)
  {
    case IF_RATE:
      *(uint32_t *)data = sspGetRate(object);
      return E_OK;
    case IF_READY:
      return dmaActive(interface->rxDma) || dmaActive(interface->txDma)
          || reg->SR & SR_BSY ? E_BUSY : E_OK;
    default:
      return E_ERROR;
  }
}
/*----------------------------------------------------------------------------*/
static enum result spiSet(void *object, enum ifOption option, const void *data)
{
  struct SpiDma *interface = object;

  switch (option)
  {
    case IF_ACQUIRE:
      return spinTryLock(&interface->lock) ? E_OK : E_BUSY;
    case IF_BLOCKING:
      dmaCallback(interface->rxDma, 0, 0);
      interface->blocking = true;
      return E_OK;
    case IF_RATE:
      sspSetRate(object, *(uint32_t *)data);
      return E_OK;
    case IF_RELEASE:
      spinUnlock(&interface->lock);
      return E_OK;
    case IF_ZEROCOPY:
      dmaCallback(interface->rxDma, dmaHandler, interface);
      interface->blocking = false;
      return E_OK;
    default:
      return E_ERROR;
  }
}
/*----------------------------------------------------------------------------*/
static uint32_t spiRead(void *object, uint8_t *buffer, uint32_t length)
{
  const uint8_t mock = 0xFF;
  struct SpiDma *interface = object;
  LPC_SSP_Type *reg = interface->parent.reg;
  void *source = (void *)&((LPC_SSP_Type *)interface->parent.reg)->DR;

  if (!length)
    return 0;

  /* Clear input FIFO */
  while (reg->SR & SR_RNE)
    (void)reg->DR;

  /* Clear timeout interrupt flags */
  reg->ICR = ICR_RORIC | ICR_RTIC;

  /* Clear DMA requests */
  reg->DMACR &= ~(DMACR_RXDMAE | DMACR_TXDMAE);
  reg->DMACR |= DMACR_RXDMAE | DMACR_TXDMAE;

  if (dmaStart(interface->rxDma, buffer, source, length) != E_OK)
    return 0;
  if (dmaStart(interface->txMockDma, source, &mock, length) != E_OK)
  {
    dmaStop(interface->rxDma);
    return 0;
  }
  if (interface->blocking)
    while (dmaActive(interface->rxDma) || reg->SR & SR_BSY);

  return length;
}
/*----------------------------------------------------------------------------*/
static uint32_t spiWrite(void *object, const uint8_t *buffer, uint32_t length)
{
  struct SpiDma *interface = object;
  LPC_SSP_Type *reg = interface->parent.reg;
  void *destination = (void *)&((LPC_SSP_Type *)interface->parent.reg)->DR;

  if (!length)
    return 0;

  /* Clear timeout interrupt flags */
  reg->ICR = ICR_RORIC | ICR_RTIC;

  /* Enable timeout interrupt when in zero copy mode */
  if (!interface->blocking)
    reg->IMSC |= IMSC_RTIM;

  /* Clear DMA requests */
  reg->DMACR &= ~(DMACR_RXDMAE | DMACR_TXDMAE);
  reg->DMACR |= DMACR_RXDMAE | DMACR_TXDMAE;

  if (dmaStart(interface->txDma, destination, buffer, length) != E_OK)
    return 0;

  if (interface->blocking)
    while (dmaActive(interface->txDma) || reg->SR & SR_BSY);

  return length;
}
