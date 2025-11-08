/*
 * spi_dma.c
 * Copyright (C) 2025 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/platform/lpc/sdma_circular.h>
#include <halm/platform/lpc/sdma_oneshot.h>
#include <halm/platform/lpc/spi_defs.h>
#include <halm/platform/lpc/spi_dma.h>
#include <halm/pm.h>
#include <assert.h>
/*----------------------------------------------------------------------------*/
#define DUMMY_FRAME 0xFF
/*----------------------------------------------------------------------------*/
static void dmaHandler(void *);
static bool dmaSetup(struct SpiDma *, uint8_t);
static void dmaSetupRx(struct Dma *, struct Dma *);
static void dmaSetupRxTx(struct Dma *, struct Dma *);
static void dmaSetupTx(struct Dma *, struct Dma *);
static enum Result getStatus(const struct SpiDma *);
static size_t transferDataDma(struct SpiDma *, const void *, void *, size_t);

#ifdef CONFIG_PLATFORM_LPC_SPI_PM
static void powerStateHandler(void *, enum PmState);
#endif
/*----------------------------------------------------------------------------*/
static enum Result spiInit(void *, const void *);
static void spiSetCallback(void *, void (*)(void *), void *);
static enum Result spiGetParam(void *, int, void *);
static enum Result spiSetParam(void *, int, const void *);
static size_t spiRead(void *, void *, size_t);
static size_t spiWrite(void *, const void *, size_t);

#ifndef CONFIG_PLATFORM_LPC_SPI_NO_DEINIT
static void spiDeinit(void *);
#else
#  define spiDeinit deletedDestructorTrap
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
  LPC_SPI_Type * const reg = interface->base.reg;

  if (interface->invoked)
  {
    reg->STAT = STAT_SSA | STAT_SSD | STAT_ENDTRANSFER;

    if (interface->callback != NULL)
      interface->callback(interface->callbackArgument);
  }
  else
    interface->invoked = true;
}
/*----------------------------------------------------------------------------*/
static bool dmaSetup(struct SpiDma *interface, uint8_t priority)
{
#if CONFIG_PLATFORM_LPC_SPI_DMA_CHAIN > 1
  const struct SdmaCircularConfig dmaConfigs[] = {
      {
          .number = CONFIG_PLATFORM_LPC_SPI_DMA_CHAIN,
          .request = sdmaGetRequestSpiRx(interface->base.channel),
          .trigger = SDMA_TRIGGER_NONE,
          .channel = SDMA_CHANNEL_AUTO,
          .priority = priority,
          .oneshot = true,
          .polarity = false,
          .silent = true
      }, {
          .number = CONFIG_PLATFORM_LPC_SPI_DMA_CHAIN,
          .request = sdmaGetRequestSpiTx(interface->base.channel),
          .trigger = SDMA_TRIGGER_NONE,
          .channel = SDMA_CHANNEL_AUTO,
          .priority = priority,
          .oneshot = true,
          .polarity = false,
          .silent = true
      }
  };
  const void * const dmaClassDescriptor = SdmaCircular;
#else
  const struct SdmaOneShotConfig dmaConfigs[] = {
      {
          .request = sdmaGetRequestSpiRx(interface->base.channel),
          .trigger = SDMA_TRIGGER_NONE,
          .channel = SDMA_CHANNEL_AUTO,
          .priority = priority,
          .polarity = false
      }, {
          .request = sdmaGetRequestSpiTx(interface->base.channel),
          .trigger = SDMA_TRIGGER_NONE,
          .channel = SDMA_CHANNEL_AUTO,
          .priority = priority,
          .polarity = false
      }
  };
  const void * const dmaClassDescriptor = SdmaOneShot;
#endif

  interface->rxDma = init(dmaClassDescriptor, &dmaConfigs[0]);
  if (interface->rxDma == NULL)
    return false;
  dmaSetCallback(interface->rxDma, dmaHandler, interface);

  interface->txDma = init(dmaClassDescriptor, &dmaConfigs[1]);
  if (interface->txDma == NULL)
    return false;
  dmaSetCallback(interface->txDma, dmaHandler, interface);

  return true;
}
/*----------------------------------------------------------------------------*/
static void dmaSetupRx(struct Dma *rx, struct Dma *tx)
{
  static const struct SdmaSettings dmaSettings[] = {
      {
          .burst = DMA_BURST_1,
          .width = DMA_WIDTH_BYTE,
          .source = {
              .stride = SDMA_STRIDE_NONE,
              .wrap = true
          },
          .destination = {
              .stride = SDMA_STRIDE_1,
              .wrap = false
          }
      }, {
          .burst = DMA_BURST_1,
          .width = DMA_WIDTH_BYTE,
          .source = {
              .stride = SDMA_STRIDE_NONE,
              .wrap = true
          },
          .destination = {
              .stride = SDMA_STRIDE_NONE,
              .wrap = true
          }
      }
  };

  dmaConfigure(rx, &dmaSettings[0]);
  dmaConfigure(tx, &dmaSettings[1]);
}
/*----------------------------------------------------------------------------*/
static void dmaSetupRxTx(struct Dma *rx, struct Dma *tx)
{
  static const struct SdmaSettings dmaSettings[] = {
      {
          .burst = DMA_BURST_1,
          .width = DMA_WIDTH_BYTE,
          .source = {
              .stride = SDMA_STRIDE_NONE,
              .wrap = true
          },
          .destination = {
              .stride = SDMA_STRIDE_1,
              .wrap = false
          }
      }, {
          .burst = DMA_BURST_1,
          .width = DMA_WIDTH_BYTE,
          .source = {
              .stride = SDMA_STRIDE_1,
              .wrap = false
          },
          .destination = {
              .stride = SDMA_STRIDE_NONE,
              .wrap = true
          }
      }
  };

  dmaConfigure(rx, &dmaSettings[0]);
  dmaConfigure(tx, &dmaSettings[1]);
}
/*----------------------------------------------------------------------------*/
static void dmaSetupTx(struct Dma *rx, struct Dma *tx)
{
  static const struct SdmaSettings dmaSettings[] = {
      {
          .burst = DMA_BURST_1,
          .width = DMA_WIDTH_BYTE,
          .source = {
              .stride = SDMA_STRIDE_NONE,
              .wrap = true
          },
          .destination = {
              .stride = SDMA_STRIDE_NONE,
              .wrap = true
          }
      }, {
          .source = {
              .stride = SDMA_STRIDE_1,
              .wrap = false
          },
          .destination = {
              .stride = SDMA_STRIDE_NONE,
              .wrap = true
          }
      }
  };

  dmaConfigure(rx, &dmaSettings[0]);
  dmaConfigure(tx, &dmaSettings[1]);
}
/*----------------------------------------------------------------------------*/
static enum Result getStatus(const struct SpiDma *interface)
{
  LPC_SPI_Type * const reg = interface->base.reg;
  enum Result res;

  if (reg->STAT & STAT_MSTIDLE)
    res = dmaStatus(interface->rxDma);
  else
    res = E_BUSY;

  return res;
}
/*----------------------------------------------------------------------------*/
#ifdef CONFIG_PLATFORM_LPC_SPI_PM
static void powerStateHandler(void *object, enum PmState state)
{
  if (state == PM_ACTIVE)
  {
    struct SpiDma * const interface = object;
    spiSetRate(&interface->base, interface->rate);
  }
}
#endif
/*----------------------------------------------------------------------------*/
static size_t transferDataDma(struct SpiDma *interface, const void *source,
    void *sink, size_t length)
{
  LPC_SPI_Type * const reg = interface->base.reg;

  if (sink == NULL)
    reg->TXCTL |= TXCTL_RXIGNORE;
  else
    reg->TXCTL &= ~TXCTL_RXIGNORE;

  interface->invoked = false;
  interface->sink = NULL;

#if CONFIG_PLATFORM_LPC_SPI_DMA_CHAIN > 1
  size_t pending = length;
  uintptr_t rxAddress = (uintptr_t)sink;
  uintptr_t txAddress = (uintptr_t)source;

  do
  {
    const size_t chunk = MIN(pending, SDMA_MAX_TRANSFER_SIZE);

    if (sink != NULL)
    {
      dmaAppend(interface->rxDma, (void *)rxAddress, (const void *)&reg->RXDAT,
          chunk);
      rxAddress += chunk;
    }

    dmaAppend(interface->txDma, (void *)&reg->TXDAT, (const void *)txAddress,
        chunk);
    txAddress += chunk;

    pending -= chunk;
  }
  while (pending);
#else
  if (sink != NULL)
    dmaAppend(interface->rxDma, sink, (const void *)&reg->RXDAT, length);
  dmaAppend(interface->txDma, (void *)&reg->TXDAT, source, length);
#endif

  enum Result res = E_OK;

  if (sink != NULL)
    res = dmaEnable(interface->rxDma);
  else
    interface->invoked = true;

  if (res == E_OK)
    res = dmaEnable(interface->txDma);

  if (res == E_OK)
  {
    if (interface->blocking)
      while ((res = getStatus(interface)) == E_BUSY);

    return res == E_OK ? length : 0;
  }
  else
  {
    if (sink != NULL)
      dmaDisable(interface->rxDma);
    return 0;
  }
}
/*----------------------------------------------------------------------------*/
static enum Result spiInit(void *object, const void *configBase)
{
  const struct SpiDmaConfig * const config = configBase;
  assert(config != NULL);

  const struct SpiBaseConfig baseConfig = {
      .cs = 0,
      .miso = config->miso,
      .mosi = config->mosi,
      .sck = config->sck,
      .channel = config->channel
  };
  struct SpiDma * const interface = object;
  enum Result res;

  /* Call base class constructor */
  if ((res = SpiBase->init(interface, &baseConfig)) != E_OK)
    return res;

  if (!dmaSetup(interface, config->priority))
    return E_ERROR;

  interface->callback = NULL;
  interface->rate = config->rate;
  interface->sink = NULL;
  interface->blocking = true;
  interface->unidir = true;

  LPC_SPI_Type * const reg = interface->base.reg;

  /* Enable master mode */
  reg->CFG = CFG_MASTER;
  /* Set frame size */
  reg->TXCTL = TXCTL_LEN(7);

  /* Set the desired data rate */
  if (!spiSetRate(&interface->base, interface->rate))
    return E_VALUE;
  /* Set SPI mode */
  spiSetMode(&interface->base, config->mode);

#ifdef CONFIG_PLATFORM_LPC_SPI_PM
  if ((res = pmRegister(powerStateHandler, interface)) != E_OK)
    return res;
#endif

  /* Enable the peripheral */
  reg->CFG |= CFG_ENABLE;

  return E_OK;
}
/*----------------------------------------------------------------------------*/
#ifndef CONFIG_PLATFORM_LPC_SPI_NO_DEINIT
static void spiDeinit(void *object)
{
  struct SpiDma * const interface = object;
  LPC_SPI_Type * const reg = interface->base.reg;

  /* Disable the peripheral */
  reg->CFG = 0;

#ifdef CONFIG_PLATFORM_LPC_SPI_PM
  pmUnregister(interface);
#endif

  deinit(interface->txDma);
  deinit(interface->rxDma);

  SpiBase->deinit(interface);
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

#ifndef CONFIG_PLATFORM_LPC_SPI_RC
  (void)data;
#endif

  switch ((enum SPIParameter)parameter)
  {
#ifdef CONFIG_PLATFORM_LPC_SPI_RC
    case IF_SPI_MODE:
      *(uint8_t *)data = spiGetMode(&interface->base);
      return E_OK;
#endif

    default:
      break;
  }

  switch ((enum IfParameter)parameter)
  {
#ifdef CONFIG_PLATFORM_LPC_SPI_RC
    case IF_RATE:
      *(uint32_t *)data = spiGetRate(object);
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

#ifndef CONFIG_PLATFORM_LPC_SPI_RC
  (void)data;
#endif

  switch ((enum SPIParameter)parameter)
  {
#ifdef CONFIG_PLATFORM_LPC_SPI_RC
    case IF_SPI_MODE:
      spiSetMode(&interface->base, *(const uint8_t *)data);
      return E_OK;
#endif

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
      interface->blocking = true;
      return E_OK;

#ifdef CONFIG_PLATFORM_LPC_SPI_RC
    case IF_RATE:
    {
      const uint32_t rate = *(const uint32_t *)data;

      if (spiSetRate(&interface->base, rate))
      {
        interface->rate = rate;
        return E_OK;
      }
      else
        return E_VALUE;
    }
#endif

    case IF_ZEROCOPY:
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
    return transferDataDma(interface, &interface->dummy, buffer, length);
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
    return transferDataDma(interface, buffer, NULL, length);
  }
  else
  {
    assert(!interface->unidir);

    dmaSetupRxTx(interface->rxDma, interface->txDma);
    return transferDataDma(interface, buffer, interface->sink, length);
  }
}
