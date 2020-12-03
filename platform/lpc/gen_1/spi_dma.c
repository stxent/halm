/*
 * spi_dma.c
 * Copyright (C) 2013 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <halm/platform/lpc/gpdma_oneshot.h>
#include <halm/platform/lpc/spi_dma.h>
#include <halm/platform/lpc/ssp_defs.h>
#include <halm/pm.h>
#include <assert.h>
/*----------------------------------------------------------------------------*/
#define DUMMY_FRAME 0xFF
/*----------------------------------------------------------------------------*/
static void dmaHandler(void *);
static bool dmaSetup(struct SpiDma *, uint8_t, uint8_t);
static void dmaSetupRx(struct Dma *, struct Dma *);
static void dmaSetupRxTx(struct Dma *, struct Dma *);
static void dmaSetupTx(struct Dma *, struct Dma *);
static enum Result getStatus(const struct SpiDma *);
static size_t transferData(struct SpiDma *, const void *, void *, size_t);

#ifdef CONFIG_PLATFORM_LPC_SSP_PM
static void powerStateHandler(void *, enum PmState);
#endif
/*----------------------------------------------------------------------------*/
static enum Result spiInit(void *, const void *);
static void spiSetCallback(void *, void (*)(void *), void *);
static enum Result spiGetParam(void *, int, void *);
static enum Result spiSetParam(void *, int, const void *);
static size_t spiRead(void *, void *, size_t);
static size_t spiWrite(void *, const void *, size_t);

#ifndef CONFIG_PLATFORM_LPC_SSP_NO_DEINIT
static void spiDeinit(void *);
#else
#define spiDeinit deletedDestructorTrap
#endif
/*----------------------------------------------------------------------------*/
const struct InterfaceClass * const SpiDma = &(const struct InterfaceClass){
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
static void dmaHandler(void *object)
{
  struct SpiDma * const interface = object;

  if (interface->callback)
  {
    if (interface->invoked)
      interface->callback(interface->callbackArgument);
    else
      interface->invoked = true;
  }
}
/*----------------------------------------------------------------------------*/
static bool dmaSetup(struct SpiDma *interface, uint8_t rxChannel,
    uint8_t txChannel)
{
  const struct GpDmaOneShotConfig dmaConfigs[] = {
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

  interface->rxDma = init(GpDmaOneShot, &dmaConfigs[0]);
  if (!interface->rxDma)
    return false;

  interface->txDma = init(GpDmaOneShot, &dmaConfigs[1]);
  if (!interface->txDma)
    return false;

  return true;
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
static void dmaSetupRxTx(struct Dma *rx, struct Dma *tx)
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
    res = E_BUSY;
  else
    res = dmaStatus(interface->rxDma);

  return res;
}
/*----------------------------------------------------------------------------*/
#ifdef CONFIG_PLATFORM_LPC_SSP_PM
static void powerStateHandler(void *object, enum PmState state)
{
  struct SpiDma * const interface = object;

  if (state == PM_ACTIVE)
    sspSetRate(object, interface->rate);
}
#endif
/*----------------------------------------------------------------------------*/
static size_t transferData(struct SpiDma *interface, const void *txSource,
    void *rxSink, size_t length)
{
  LPC_SSP_Type * const reg = interface->base.reg;

  /* Clear DMA requests */
  reg->DMACR = 0;
  reg->DMACR = DMACR_RXDMAE | DMACR_TXDMAE;

  interface->invoked = false;
  interface->sink = 0;
  dmaAppend(interface->rxDma, rxSink, (const void *)&reg->DR, length);
  dmaAppend(interface->txDma, (void *)&reg->DR, txSource, length);

  if (dmaEnable(interface->rxDma) != E_OK)
  {
    goto error;
  }
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
  if ((res = SspBase->init(interface, &baseConfig)) != E_OK)
    return res;

  /* RX channel should have higher DMA priority */
  const bool channelPair = config->dma[0] > config->dma[1];
  const uint8_t rxChannel = config->dma[channelPair];
  const uint8_t txChannel = config->dma[!channelPair];

  if (!dmaSetup(interface, rxChannel, txChannel))
    return E_ERROR;

  interface->callback = 0;
  interface->rate = config->rate;
  interface->sink = 0;
  interface->blocking = true;
  interface->unidir = true;

  LPC_SSP_Type * const reg = interface->base.reg;
  uint32_t controlValue = 0;

  /* Set frame size */
  controlValue |= CR0_DSS(8);

  /* Set mode of the interface */
  if (config->mode & 0x01)
    controlValue |= CR0_CPHA;
  if (config->mode & 0x02)
    controlValue |= CR0_CPOL;

  reg->CR0 = controlValue;
  /* Disable all interrupts */
  reg->IMSC = 0;

  /* Set the desired data rate */
  sspSetRate(object, interface->rate);

#ifdef CONFIG_PLATFORM_LPC_SSP_PM
  if ((res = pmRegister(powerStateHandler, interface)) != E_OK)
    return res;
#endif

  /* Enable the peripheral */
  reg->CR1 = CR1_SSE;

  return E_OK;
}
/*----------------------------------------------------------------------------*/
#ifndef CONFIG_PLATFORM_LPC_SSP_NO_DEINIT
static void spiDeinit(void *object)
{
  struct SpiDma * const interface = object;
  LPC_SSP_Type * const reg = interface->base.reg;

  /* Disable the peripheral */
  reg->CR1 = 0;

#ifdef CONFIG_PLATFORM_LPC_SSP_PM
  pmUnregister(interface);
#endif

  deinit(interface->txDma);
  deinit(interface->rxDma);

  SspBase->deinit(interface);
}
#endif
/*----------------------------------------------------------------------------*/
static void spiSetCallback(void *object, void (*callback)(void *),
    void *argument)
{
  struct SpiDma * const interface = object;

  interface->callbackArgument = argument;
  interface->callback = callback;
}
/*----------------------------------------------------------------------------*/
static enum Result spiGetParam(void *object, int parameter, void *data)
{
  struct SpiDma * const interface = object;

#ifndef CONFIG_PLATFORM_LPC_SSP_RC
  (void)data;
#endif

  switch ((enum IfParameter)parameter)
  {
#ifdef CONFIG_PLATFORM_LPC_SSP_RC
    case IF_RATE:
      *(uint32_t *)data = sspGetRate(object);
      return E_OK;
#endif

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
static enum Result spiSetParam(void *object, int parameter, const void *data)
{
  struct SpiDma * const interface = object;

#ifndef CONFIG_PLATFORM_LPC_SSP_RC
  (void)data;
#endif

  switch ((enum SpiParameter)parameter)
  {
    case IF_SPI_BIDIRECTIONAL:
      interface->unidir = false;
      return E_OK;

    case IF_SPI_UNIDIRECTIONAL:
      interface->unidir = true;
      return E_OK;

    default:
      break;
  }

  switch ((enum IfParameter)parameter)
  {
    case IF_BLOCKING:
      dmaSetCallback(interface->rxDma, 0, 0);
      dmaSetCallback(interface->txDma, 0, 0);
      interface->blocking = true;
      return E_OK;

#ifdef CONFIG_PLATFORM_LPC_SSP_RC
    case IF_RATE:
      interface->rate = *(const uint32_t *)data;
      sspSetRate(object, interface->rate);
      return E_OK;
#endif

    case IF_ZEROCOPY:
      dmaSetCallback(interface->rxDma, dmaHandler, interface);
      dmaSetCallback(interface->txDma, dmaHandler, interface);
      interface->blocking = false;
      return E_OK;

    default:
      return E_INVALID;
  }
}
/*----------------------------------------------------------------------------*/
static size_t spiRead(void *object, void *buffer, size_t length)
{
  if (!length)
    return 0;

  struct SpiDma * const interface = object;

  if (interface->unidir)
  {
    interface->dummy = DUMMY_FRAME;
    dmaSetupRx(interface->rxDma, interface->txDma);
    return transferData(interface, &interface->dummy, buffer, length);
  }
  else
  {
    interface->sink = buffer;
    return length;
  }
}
/*----------------------------------------------------------------------------*/
static size_t spiWrite(void *object, const void *buffer, size_t length)
{
  if (!length)
    return 0;

  struct SpiDma * const interface = object;

  if (!interface->sink)
  {
    dmaSetupTx(interface->rxDma, interface->txDma);
    return transferData(interface, buffer, &interface->dummy, length);
  }
  else
  {
    assert(!interface->unidir);

    dmaSetupRxTx(interface->rxDma, interface->txDma);
    return transferData(interface, buffer, interface->sink, length);
  }
}
