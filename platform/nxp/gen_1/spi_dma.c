/*
 * spi_dma.c
 * Copyright (C) 2013 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <assert.h>
#include <platform/nxp/gpdma.h>
#include <platform/nxp/spi_dma.h>
#include <platform/nxp/ssp_defs.h>
/*----------------------------------------------------------------------------*/
#define DUMMY_FRAME 0xFF
/*----------------------------------------------------------------------------*/
static void dmaHandler(void *);
static void dmaRxSetup(struct SpiDma *);
static enum result dmaSetup(struct SpiDma *, uint8_t, uint8_t);
static void dmaTxSetup(struct SpiDma *);
static enum result getStatus(const struct SpiDma *);
/*----------------------------------------------------------------------------*/
static enum result spiInit(void *, const void *);
static void spiDeinit(void *);
static enum result spiCallback(void *, void (*)(void *), void *);
static enum result spiGet(void *, enum ifOption, void *);
static enum result spiSet(void *, enum ifOption, const void *);
static size_t spiRead(void *, void *, size_t);
static size_t spiWrite(void *, const void *, size_t);
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
const struct InterfaceClass * const SpiDma = &spiTable;
/*----------------------------------------------------------------------------*/
static void dmaHandler(void *object)
{
  struct SpiDma * const interface = object;

  if (interface->callback)
    interface->callback(interface->callbackArgument);
}
/*----------------------------------------------------------------------------*/
static void dmaRxSetup(struct SpiDma *interface)
{
  static const struct GpDmaRuntimeConfig configs[] = {
      {
          .source.increment = false,
          .destination.increment = true
      },
      {
          .source.increment = false,
          .destination.increment = false
      }
  };

  /* Return values are ignored, increment setup cannot fail */
  dmaReconfigure(interface->rxDma, &configs[0]);
  dmaReconfigure(interface->txDma, &configs[1]);
}
/*----------------------------------------------------------------------------*/
static enum result dmaSetup(struct SpiDma *interface, uint8_t rxChannel,
    uint8_t txChannel)
{
  const struct GpDmaConfig configs[] = {
      {
          .event = GPDMA_SSP0_RX + interface->base.channel,
          .channel = rxChannel,
          .source.increment = false,
          .destination.increment = true,
          .type = GPDMA_TYPE_P2M,
          .burst = DMA_BURST_4,
          .width = DMA_WIDTH_BYTE
      }, {
          .event = GPDMA_SSP0_TX + interface->base.channel,
          .channel = txChannel,
          .source.increment = true,
          .destination.increment = false,
          .type = GPDMA_TYPE_M2P,
          .burst = DMA_BURST_4,
          .width = DMA_WIDTH_BYTE
      }
  };

  interface->rxDma = init(GpDma, &configs[0]);
  if (!interface->rxDma)
    return E_ERROR;

  interface->txDma = init(GpDma, &configs[1]);
  if (!interface->txDma)
    return E_ERROR;

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static void dmaTxSetup(struct SpiDma *interface)
{
  static const struct GpDmaRuntimeConfig configs[] = {
      {
          .source.increment = false,
          .destination.increment = false
      },
      {
          .source.increment = true,
          .destination.increment = false
      }
  };

  /* Return values are ignored, increment setup cannot fail */
  dmaReconfigure(interface->rxDma, &configs[0]);
  dmaReconfigure(interface->txDma, &configs[1]);
}
/*----------------------------------------------------------------------------*/
static enum result getStatus(const struct SpiDma *interface)
{
  LPC_SSP_Type * const reg = interface->base.reg;
  enum result res;

  if (reg->SR & SR_BSY)
    return E_BUSY;
  if ((res = dmaStatus(interface->txDma)) != E_OK)
    return res;
  if ((res = dmaStatus(interface->rxDma)) != E_OK)
    return res;

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static enum result spiInit(void *object, const void *configBase)
{
  const struct SpiDmaConfig * const config = configBase;
  const struct SspBaseConfig baseConfig = {
      .channel = config->channel,
      .miso = config->miso,
      .mosi = config->mosi,
      .sck = config->sck,
      .cs = 0
  };
  struct SpiDma * const interface = object;
  enum result res;

  assert(config->dma[0] != config->dma[1]);

  /* Call base class constructor */
  if ((res = SspBase->init(object, &baseConfig)) != E_OK)
    return res;

  const bool channelPair = config->dma[0] > config->dma[1];
  const uint8_t rxChannel = config->dma[channelPair];
  const uint8_t txChannel = config->dma[!channelPair];

  if ((res = dmaSetup(interface, rxChannel, txChannel)) != E_OK)
    return res;

  interface->blocking = true;
  interface->callback = 0;

  LPC_SSP_Type * const reg = interface->base.reg;
  uint32_t controlValue = 0;

  /* Set frame size */
  controlValue |= CR0_DSS(8);

  /* Set mode for SPI interface */
  if (config->mode & 0x01)
    controlValue |= CR0_CPHA;
  if (config->mode & 0x02)
    controlValue |= CR0_CPOL;

  reg->CR0 = controlValue;
  /* Disable all interrupts */
  reg->IMSC = 0;

  /* Try to set the desired data rate */
  sspSetRate(object, config->rate);

  /* Enable peripheral */
  reg->CR1 = CR1_SSE;

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static void spiDeinit(void *object)
{
  struct SpiDma * const interface = object;
  LPC_SSP_Type * const reg = interface->base.reg;

  /* Disable peripheral */
  reg->CR1 = 0;

  deinit(interface->txDma);
  deinit(interface->rxDma);

  SspBase->deinit(interface);
}
/*----------------------------------------------------------------------------*/
static enum result spiCallback(void *object, void (*callback)(void *),
    void *argument)
{
  struct SpiDma * const interface = object;

  interface->callbackArgument = argument;
  interface->callback = callback;
  return E_OK;
}
/*----------------------------------------------------------------------------*/
static enum result spiGet(void *object, enum ifOption option, void *data)
{
  struct SpiDma * const interface = object;

  switch (option)
  {
    case IF_RATE:
      *(uint32_t *)data = sspGetRate(object);
      return E_OK;

    case IF_STATUS:
      /*
       * Previous type of operation should be performed when a direct memory
       * access error occurs or a status of the interface would repeatedly
       * return last error code.
       */
      return getStatus(interface);

    default:
      return E_ERROR;
  }
}
/*----------------------------------------------------------------------------*/
static enum result spiSet(void *object, enum ifOption option, const void *data)
{
  struct SpiDma * const interface = object;

  switch (option)
  {
    case IF_BLOCKING:
      dmaCallback(interface->rxDma, 0, 0);
      interface->blocking = true;
      return E_OK;

    case IF_RATE:
      sspSetRate(object, *(const uint32_t *)data);
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
static size_t spiRead(void *object, void *buffer, size_t length)
{
  struct SpiDma * const interface = object;
  LPC_SSP_Type * const reg = interface->base.reg;
  enum result res;

  if (!length)
    return 0;

  /* Clear timeout interrupt flags */
  reg->ICR = ICR_RORIC | ICR_RTIC;

  /* Clear DMA requests */
  reg->DMACR = 0;
  reg->DMACR = DMACR_RXDMAE | DMACR_TXDMAE;

  dmaRxSetup(interface);

  res = dmaStart(interface->rxDma, buffer, (const void *)&reg->DR, length);
  if (res != E_OK)
    return 0;

  interface->dummy = DUMMY_FRAME;
  res = dmaStart(interface->txDma, (void *)&reg->DR, &interface->dummy, length);
  if (res != E_OK)
  {
    dmaStop(interface->rxDma);
    return 0;
  }

  if (interface->blocking)
  {
    while ((res = getStatus(interface)) == E_BUSY)
      barrier();
  }

  return res == E_OK ? length : 0;
}
/*----------------------------------------------------------------------------*/
static size_t spiWrite(void *object, const void *buffer, size_t length)
{
  struct SpiDma * const interface = object;
  LPC_SSP_Type * const reg = interface->base.reg;
  enum result res;

  if (!length)
    return 0;

  /* Clear timeout interrupt flags */
  reg->ICR = ICR_RORIC | ICR_RTIC;

  /* Clear DMA requests */
  reg->DMACR = 0;
  reg->DMACR = DMACR_RXDMAE | DMACR_TXDMAE;

  dmaTxSetup(interface);

  res = dmaStart(interface->rxDma, &interface->dummy, (const void *)&reg->DR,
      length);
  if (res != E_OK)
    return 0;

  res = dmaStart(interface->txDma, (void *)&reg->DR, buffer, length);
  if (res != E_OK)
  {
    dmaStop(interface->rxDma);
    return 0;
  }

  if (interface->blocking)
  {
    while ((res = getStatus(interface)) == E_BUSY)
      barrier();
  }

  return res == E_OK ? length : 0;
}
