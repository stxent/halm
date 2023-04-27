/*
 * spi_dma.c
 * Copyright (C) 2013 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/platform/lpc/gpdma_circular.h>
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

  if (interface->callback != NULL)
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
#if CONFIG_PLATFORM_LPC_SPI_DMA_CHAIN > 1
  const struct GpDmaCircularConfig dmaConfigs[] = {
      {
          .number = CONFIG_PLATFORM_LPC_SPI_DMA_CHAIN,
          .event = GPDMA_SSP0_RX + interface->base.channel,
          .type = GPDMA_TYPE_P2M,
          .channel = rxChannel,
          .oneshot = true,
          .silent = true
      }, {
          .number = CONFIG_PLATFORM_LPC_SPI_DMA_CHAIN,
          .event = GPDMA_SSP0_TX + interface->base.channel,
          .type = GPDMA_TYPE_M2P,
          .channel = txChannel,
          .oneshot = true,
          .silent = true
      }
  };
  const void * const dmaClassDescriptor = GpDmaCircular;
#else
  const struct GpDmaOneShotConfig dmaConfigs[] = {
      {
          .event = GPDMA_SSP0_RX + interface->base.channel,
          .type = GPDMA_TYPE_P2M,
          .channel = rxChannel
      }, {
          .event = GPDMA_SSP0_TX + interface->base.channel,
          .type = GPDMA_TYPE_M2P,
          .channel = txChannel
      }
  };
  const void * const dmaClassDescriptor = GpDmaOneShot;
#endif

  interface->rxDma = init(dmaClassDescriptor, &dmaConfigs[0]);
  if (interface->rxDma == NULL)
    return false;

  interface->txDma = init(dmaClassDescriptor, &dmaConfigs[1]);
  if (interface->txDma == NULL)
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
      }, {
          .source = {
              .burst = DMA_BURST_4,
              .width = DMA_WIDTH_BYTE,
              .increment = false
          },
          .destination = {
              .burst = DMA_BURST_1,
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
      }, {
          .source = {
              .burst = DMA_BURST_4,
              .width = DMA_WIDTH_BYTE,
              .increment = true
          },
          .destination = {
              .burst = DMA_BURST_1,
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
      }, {
          .source = {
              .burst = DMA_BURST_4,
              .width = DMA_WIDTH_BYTE,
              .increment = true
          },
          .destination = {
              .burst = DMA_BURST_1,
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
static size_t transferData(struct SpiDma *interface, const void *source,
    void *sink, size_t length)
{
  LPC_SSP_Type * const reg = interface->base.reg;

  /* Clear DMA requests */
  reg->DMACR = 0;
  reg->DMACR = DMACR_RXDMAE | DMACR_TXDMAE;

  interface->invoked = false;
  interface->sink = NULL;

#if CONFIG_PLATFORM_LPC_SPI_DMA_CHAIN > 1
  size_t pending = length;
  uintptr_t rxAddress = (uintptr_t)sink;
  uintptr_t txAddress = (uintptr_t)source;

  do
  {
    const size_t chunk = MIN(pending, GPDMA_MAX_TRANSFER_SIZE);

    dmaAppend(interface->rxDma, (void *)rxAddress, (const void *)&reg->DR,
        chunk);
    dmaAppend(interface->txDma, (void *)&reg->DR, (const void *)txAddress,
        chunk);

    rxAddress += chunk;
    txAddress += chunk;
    pending -= chunk;
  }
  while (pending);
#else
  dmaAppend(interface->rxDma, sink, (const void *)&reg->DR, length);
  dmaAppend(interface->txDma, (void *)&reg->DR, source, length);
#endif

  if (dmaEnable(interface->rxDma) != E_OK)
  {
    return 0;
  }
  if (dmaEnable(interface->txDma) != E_OK)
  {
    dmaDisable(interface->rxDma);
    return 0;
  }

  enum Result res = E_OK;

  if (interface->blocking)
    while ((res = getStatus(interface)) == E_BUSY);

  return res == E_OK ? length : 0;
}
/*----------------------------------------------------------------------------*/
static enum Result spiInit(void *object, const void *configBase)
{
  const struct SpiDmaConfig * const config = configBase;
  assert(config != NULL);

  const struct SspBaseConfig baseConfig = {
      .cs = 0,
      .miso = config->miso,
      .mosi = config->mosi,
      .sck = config->sck,
      .channel = config->channel
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

  interface->callback = NULL;
  interface->rate = config->rate;
  interface->sink = NULL;
  interface->blocking = true;
  interface->unidir = true;

  LPC_SSP_Type * const reg = interface->base.reg;

  /* Set frame size */
  reg->CR0 = CR0_DSS(8);
  /* Disable all interrupts */
  reg->IMSC = 0;

  /* Set SPI mode */
  sspSetMode(&interface->base, config->mode);
  /* Set the desired data rate */
  sspSetRate(&interface->base, interface->rate);

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

  switch ((enum SPIParameter)parameter)
  {
#ifdef CONFIG_PLATFORM_LPC_SSP_RC
    case IF_SPI_MODE:
      *(uint8_t *)data = sspGetMode(&interface->base);
      return E_OK;
#endif

    default:
      break;
  }

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

  switch ((enum SPIParameter)parameter)
  {
    case IF_SPI_MODE:
      sspSetMode(&interface->base, *(const uint8_t *)data);
      return E_OK;

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
      dmaSetCallback(interface->rxDma, NULL, NULL);
      dmaSetCallback(interface->txDma, NULL, NULL);
      interface->blocking = true;
      return E_OK;

#ifdef CONFIG_PLATFORM_LPC_SSP_RC
    case IF_RATE:
      interface->rate = *(const uint32_t *)data;
      sspSetRate(&interface->base, interface->rate);
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

  if (interface->sink == NULL)
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
