/*
 * spi_dma.c
 * Copyright (C) 2023 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/platform/numicro/pdma_oneshot.h>
#include <halm/platform/numicro/spi_defs.h>
#include <halm/platform/numicro/spi_dma.h>
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

#ifdef CONFIG_PLATFORM_NUMICRO_SPI_PM
static void powerStateHandler(void *, enum PmState);
#endif
/*----------------------------------------------------------------------------*/
static enum Result spiInit(void *, const void *);
static void spiSetCallback(void *, void (*)(void *), void *);
static enum Result spiGetParam(void *, int, void *);
static enum Result spiSetParam(void *, int, const void *);
static size_t spiRead(void *, void *, size_t);
static size_t spiWrite(void *, const void *, size_t);

#ifndef CONFIG_PLATFORM_NUMICRO_SPI_NO_DEINIT
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
  NM_SPI_Type * const reg = interface->base.reg;

  if (interface->invoked)
  {
    reg->PDMACTL = 0;

    if (interface->callback != NULL)
      interface->callback(interface->callbackArgument);
  }
  else
    interface->invoked = true;
}
/*----------------------------------------------------------------------------*/
static bool dmaSetup(struct SpiDma *interface, uint8_t rxChannel,
    uint8_t txChannel)
{
  const struct PdmaOneShotConfig dmaConfigs[] = {
      {
          .event = pdmaGetEventSpiRx(interface->base.channel),
          .channel = rxChannel
      }, {
          .event = pdmaGetEventSpiTx(interface->base.channel),
          .channel = txChannel
      }
  };

  interface->rxDma = init(PdmaOneShot, &dmaConfigs[0]);
  if (interface->rxDma == NULL)
    return false;

  interface->txDma = init(PdmaOneShot, &dmaConfigs[1]);
  if (interface->txDma == NULL)
    return false;

  dmaSetCallback(interface->rxDma, dmaHandler, interface);
  dmaSetCallback(interface->txDma, dmaHandler, interface);
  return true;
}
/*----------------------------------------------------------------------------*/
static void dmaSetupRx(struct Dma *rx, struct Dma *tx)
{
  static const struct PdmaSettings dmaSettings[] = {
      {
          .burst = DMA_BURST_1,
          .priority = DMA_PRIORITY_FIXED,
          .width = DMA_WIDTH_BYTE,
          .source = {
              .increment = false
          },
          .destination = {
              .increment = true
          }
      }, {
          .burst = DMA_BURST_1,
          .priority = DMA_PRIORITY_FIXED,
          .width = DMA_WIDTH_BYTE,
          .source = {
              .increment = false
          },
          .destination = {
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
  static const struct PdmaSettings dmaSettings[] = {
      {
          .burst = DMA_BURST_1,
          .priority = DMA_PRIORITY_FIXED,
          .width = DMA_WIDTH_BYTE,
          .source = {
              .increment = false
          },
          .destination = {
              .increment = true
          }
      }, {
          .burst = DMA_BURST_1,
          .priority = DMA_PRIORITY_FIXED,
          .width = DMA_WIDTH_BYTE,
          .source = {
              .increment = true
          },
          .destination = {
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
  static const struct PdmaSettings dmaSettings[] = {
      {
          .burst = DMA_BURST_1,
          .priority = DMA_PRIORITY_FIXED,
          .width = DMA_WIDTH_BYTE,
          .source = {
              .increment = false
          },
          .destination = {
              .increment = false
          }
      }, {
          .burst = DMA_BURST_1,
          .priority = DMA_PRIORITY_FIXED,
          .width = DMA_WIDTH_BYTE,
          .source = {
              .increment = true
          },
          .destination = {
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
  NM_SPI_Type * const reg = interface->base.reg;
  enum Result res;

  if (reg->STATUS & STATUS_BUSY)
    res = E_BUSY;
  else
    res = dmaStatus(interface->rxDma);

  return res;
}
/*----------------------------------------------------------------------------*/
#ifdef CONFIG_PLATFORM_NUMICRO_SPI_PM
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
static size_t transferData(struct SpiDma *interface, const void *txSource,
    void *rxSink, size_t length)
{
  NM_SPI_Type * const reg = interface->base.reg;

  interface->invoked = false;
  interface->sink = NULL;

  reg->PDMACTL = PDMACTL_RXPDMAEN | PDMACTL_TXPDMAEN;
  dmaAppend(interface->rxDma, rxSink, (const void *)&reg->RX, length);
  dmaAppend(interface->txDma, (void *)&reg->TX, txSource, length);

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

  if (!dmaSetup(interface, config->dma[0], config->dma[1]))
    return E_ERROR;

  interface->callback = NULL;
  interface->rate = config->rate;
  interface->sink = NULL;
  interface->blocking = true;
  interface->unidir = true;

  NM_SPI_Type * const reg = interface->base.reg;

  /* Disable the controller before changing the configuration */
  reg->CTL &= ~CTL_SPIEN;
  while (reg->STATUS & STATUS_SPIENSTS);

  /* Set data width and word timeout, enable interrupts */
  reg->CTL = CTL_SUSPITV(0) | CTL_DWIDTH(8) | CTL_UNITIEN;
  /* Configure Slave Select output */
  reg->SSCTL = SSCTL_SS | SSCTL_AUTOSS;
  /* Clear FIFO */
  reg->FIFOCTL = FIFOCTL_RXRST | FIFOCTL_TXRST;
  /* Clear pending interrupt flags */
  reg->STATUS = STATUS_UNITIF | STATUS_RXOVIF | STATUS_RXTOIF;

  /* Set the desired data rate */
  spiSetRate(&interface->base, interface->rate);
  /* Set SPI mode */
  spiSetMode(&interface->base, config->mode);

  /* Reset other control registers */
  reg->PDMACTL = 0;
  reg->I2SCTL = 0;
  reg->I2SCLK = 0;

#ifdef CONFIG_PLATFORM_NUMICRO_SPI_PM
  if ((res = pmRegister(powerStateHandler, interface)) != E_OK)
    return res;
#endif

  /* Enable the peripheral */
  reg->CTL |= CTL_SPIEN;

  return E_OK;
}
/*----------------------------------------------------------------------------*/
#ifndef CONFIG_PLATFORM_NUMICRO_SPI_NO_DEINIT
static void spiDeinit(void *object)
{
  struct SpiDma * const interface = object;
  NM_SPI_Type * const reg = interface->base.reg;

  /* Disable the peripheral */
  reg->CTL &= ~CTL_SPIEN;

#ifdef CONFIG_PLATFORM_NUMICRO_SPI_PM
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

#ifndef CONFIG_PLATFORM_NUMICRO_SPI_RC
  (void)data;
#endif

  switch ((enum SPIParameter)parameter)
  {
#ifdef CONFIG_PLATFORM_NUMICRO_SPI_RC
    case IF_SPI_MODE:
      *(uint8_t *)data = spiGetMode(&interface->base);
      return E_OK;
#endif

    default:
      break;
  }

  switch ((enum IfParameter)parameter)
  {
#ifdef CONFIG_PLATFORM_NUMICRO_SPI_RC
    case IF_RATE:
      *(uint32_t *)data = spiGetRate(object);
      return E_OK;
#endif

    case IF_STATUS:
      return getStatus(interface);

    default:
      return E_INVALID;
  }
}
/*----------------------------------------------------------------------------*/
static enum Result spiSetParam(void *object, int parameter, const void *data)
{
  struct SpiDma * const interface = object;

#ifndef CONFIG_PLATFORM_NUMICRO_SPI_RC
  (void)data;
#endif

  switch ((enum SPIParameter)parameter)
  {
#ifdef CONFIG_PLATFORM_NUMICRO_SPI_RC
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

#ifdef CONFIG_PLATFORM_NUMICRO_SPI_RC
    case IF_RATE:
      interface->rate = *(const uint32_t *)data;
      spiSetRate(&interface->base, interface->rate);
      return E_OK;
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
