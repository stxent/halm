/*
 * spi_dma.c
 * Copyright (C) 2013 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <assert.h>
#include <halm/platform/nxp/gpdma.h>
#include <halm/platform/nxp/spi_dma.h>
#include <halm/platform/nxp/ssp_defs.h>
#include <halm/pm.h>
/*----------------------------------------------------------------------------*/
#define DUMMY_FRAME 0xFF
/*----------------------------------------------------------------------------*/
static void dmaHandler(void *);
static enum Result dmaSetup(struct SpiDma *, uint8_t, uint8_t);
static void dmaSetupRx(struct Dma *, struct Dma *);
static void dmaSetupTx(struct Dma *, struct Dma *);
static enum Result getStatus(const struct SpiDma *);
/*----------------------------------------------------------------------------*/
#ifdef CONFIG_PLATFORM_NXP_SSP_PM
static void powerStateHandler(void *, enum PmState);
#endif
/*----------------------------------------------------------------------------*/
static enum Result spiInit(void *, const void *);
static void spiDeinit(void *);
static enum Result spiSetCallback(void *, void (*)(void *), void *);
static enum Result spiGetParam(void *, enum IfParameter, void *);
static enum Result spiSetParam(void *, enum IfParameter, const void *);
static size_t spiRead(void *, void *, size_t);
static size_t spiWrite(void *, const void *, size_t);
/*----------------------------------------------------------------------------*/
static const struct InterfaceClass spiTable = {
    .size = sizeof(struct SpiDma),
    .init = spiInit,
    .deinit = spiDeinit,

    .setCallback = spiSetCallback,
    .getParam = spiGetParam,
    .setParam = spiSetParam,
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
static enum Result dmaSetup(struct SpiDma *interface, uint8_t rxChannel,
    uint8_t txChannel)
{
  const struct GpDmaConfig dmaConfigs[] = {
      {
          .event = GPDMA_SSP0_RX + interface->base.channel,
          .type = GPDMA_TYPE_P2M,
          .channel = rxChannel
      },
      {
          .event = GPDMA_SSP0_TX + interface->base.channel,
          .type = GPDMA_TYPE_M2P,
          .channel = txChannel
      }
  };

  interface->rxDma = init(GpDma, &dmaConfigs[0]);
  if (!interface->rxDma)
    return E_ERROR;

  interface->txDma = init(GpDma, &dmaConfigs[1]);
  if (!interface->txDma)
    return E_ERROR;

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static void dmaSetupRx(struct Dma *rx, struct Dma *tx)
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
              .increment = false
          },
          .destination = {
              .burst = DMA_BURST_4,
              .width = DMA_WIDTH_BYTE,
              .increment = false
          }
      }
  };

  dmaConfigure(rx, &dmaSettings[0]);
  dmaConfigure(tx, &dmaSettings[1]);
}
/*----------------------------------------------------------------------------*/
static void dmaSetupTx(struct Dma *rx, struct Dma *tx)
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
              .increment = false
          }
      },
      {
          .source = {
              .burst = DMA_BURST_4,
              .width = DMA_WIDTH_BYTE,
              .increment = true
          },
          .destination = {
              .burst = DMA_BURST_4,
              .width = DMA_WIDTH_BYTE,
              .increment = false
          }
      }
  };

  dmaConfigure(rx, &dmaSettings[0]);
  dmaConfigure(tx, &dmaSettings[1]);
}
/*----------------------------------------------------------------------------*/
static enum Result getStatus(const struct SpiDma *interface)
{
  LPC_SSP_Type * const reg = interface->base.reg;
  enum Result res;

  if (reg->SR & SR_BSY)
    return E_BUSY;
  if ((res = dmaStatus(interface->rxDma)) != E_OK)
    return res;

  return E_OK;
}
/*----------------------------------------------------------------------------*/
#ifdef CONFIG_PLATFORM_NXP_SSP_PM
static void powerStateHandler(void *object, enum PmState state)
{
  struct SpiDma * const interface = object;

  if (state == PM_ACTIVE)
    sspSetRate(object, interface->rate);
}
#endif
/*----------------------------------------------------------------------------*/
static enum Result spiInit(void *object, const void *configBase)
{
  const struct SpiDmaConfig * const config = configBase;
  assert(config);

  const struct SspBaseConfig baseConfig = {
      .channel = config->channel,
      .miso = config->miso,
      .mosi = config->mosi,
      .sck = config->sck,
      .cs = 0
  };
  struct SpiDma * const interface = object;
  enum Result res;

  assert(config->dma[0] != config->dma[1]);

  /* Call base class constructor */
  if ((res = SspBase->init(object, &baseConfig)) != E_OK)
    return res;

  const bool channelPair = config->dma[0] > config->dma[1];
  const uint8_t rxChannel = config->dma[!channelPair];
  const uint8_t txChannel = config->dma[channelPair];

  if ((res = dmaSetup(interface, rxChannel, txChannel)) != E_OK)
    return res;

  interface->callback = 0;
  interface->rate = config->rate;
  interface->blocking = true;

  LPC_SSP_Type * const reg = interface->base.reg;
  uint32_t controlValue = 0;

  /* Set frame size */
  controlValue |= CR0_DSS(8);

  /* Set mode for the interface */
  if (config->mode & 0x01)
    controlValue |= CR0_CPHA;
  if (config->mode & 0x02)
    controlValue |= CR0_CPOL;

  reg->CR0 = controlValue;
  /* Disable all interrupts */
  reg->IMSC = 0;

  /* Try to set the desired data rate */
  sspSetRate(object, interface->rate);

#ifdef CONFIG_PLATFORM_NXP_SSP_PM
  if ((res = pmRegister(powerStateHandler, interface)) != E_OK)
    return res;
#endif

  /* Enable the peripheral */
  reg->CR1 = CR1_SSE;

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static void spiDeinit(void *object)
{
  struct SpiDma * const interface = object;
  LPC_SSP_Type * const reg = interface->base.reg;

  /* Disable the peripheral */
  reg->CR1 = 0;

#ifdef CONFIG_PLATFORM_NXP_SSP_PM
  pmUnregister(interface);
#endif

  deinit(interface->txDma);
  deinit(interface->rxDma);

  SspBase->deinit(interface);
}
/*----------------------------------------------------------------------------*/
static enum Result spiSetCallback(void *object, void (*callback)(void *),
    void *argument)
{
  struct SpiDma * const interface = object;

  interface->callbackArgument = argument;
  interface->callback = callback;
  return E_OK;
}
/*----------------------------------------------------------------------------*/
static enum Result spiGetParam(void *object, enum IfParameter parameter,
    void *data)
{
  struct SpiDma * const interface = object;

  switch (parameter)
  {
    case IF_RATE:
      *(uint32_t *)data = interface->rate;
      return E_OK;

    case IF_STATUS:
      /*
       * Previous type of operation should be performed when a direct memory
       * access error occurs or a status of the interface would repeatedly
       * return last error code.
       */
      return getStatus(interface);

    default:
      return E_INVALID;
  }
}
/*----------------------------------------------------------------------------*/
static enum Result spiSetParam(void *object, enum IfParameter parameter,
    const void *data)
{
  struct SpiDma * const interface = object;

  switch (parameter)
  {
    case IF_BLOCKING:
      dmaSetCallback(interface->rxDma, 0, 0);
      interface->blocking = true;
      return E_OK;

    case IF_RATE:
      interface->rate = *(const uint32_t *)data;
      sspSetRate(object, interface->rate);
      return E_OK;

    case IF_ZEROCOPY:
      dmaSetCallback(interface->rxDma, dmaHandler, interface);
      interface->blocking = false;
      return E_OK;

    default:
      return E_INVALID;
  }
}
/*----------------------------------------------------------------------------*/
static size_t spiRead(void *object, void *buffer, size_t length)
{
  struct SpiDma * const interface = object;
  LPC_SSP_Type * const reg = interface->base.reg;

  if (!length)
    return 0;

  /* Clear timeout interrupt flags */
  reg->ICR = ICR_RORIC | ICR_RTIC;

  /* Clear DMA requests */
  reg->DMACR = 0;
  reg->DMACR = DMACR_RXDMAE | DMACR_TXDMAE;

  dmaSetupRx(interface->rxDma, interface->txDma);
  interface->dummy = DUMMY_FRAME;

  const void * const dummy = &interface->dummy;
  void * const target = (void *)&reg->DR;

  dmaAppend(interface->rxDma, buffer, target, length);
  dmaAppend(interface->txDma, target, dummy, length);

  if (dmaEnable(interface->rxDma) != E_OK)
    goto error;
  if (dmaEnable(interface->txDma) != E_OK)
  {
    dmaDisable(interface->rxDma);
    goto error;
  }

  enum Result res = E_OK;

  if (interface->blocking)
    while ((res = getStatus(interface)) == E_BUSY);

  return res == E_OK ? length : 0;

error:
  dmaClear(interface->txDma);
  dmaClear(interface->rxDma);
  return 0;
}
/*----------------------------------------------------------------------------*/
static size_t spiWrite(void *object, const void *buffer, size_t length)
{
  struct SpiDma * const interface = object;
  LPC_SSP_Type * const reg = interface->base.reg;

  if (!length)
    return 0;

  /* Clear timeout interrupt flags */
  reg->ICR = ICR_RORIC | ICR_RTIC;

  /* Clear DMA requests */
  reg->DMACR = 0;
  reg->DMACR = DMACR_RXDMAE | DMACR_TXDMAE;

  dmaSetupTx(interface->rxDma, interface->txDma);

  void * const dummy = &interface->dummy;
  void * const target = (void *)&reg->DR;

  dmaAppend(interface->rxDma, dummy, target, length);
  dmaAppend(interface->txDma, target, buffer, length);

  if (dmaEnable(interface->rxDma) != E_OK)
    goto error;
  if (dmaEnable(interface->txDma) != E_OK)
  {
    dmaDisable(interface->rxDma);
    goto error;
  }

  enum Result res = E_OK;

  if (interface->blocking)
    while ((res = getStatus(interface)) == E_BUSY);

  return res == E_OK ? length : 0;

error:
  dmaClear(interface->txDma);
  dmaClear(interface->rxDma);
  return 0;
}
