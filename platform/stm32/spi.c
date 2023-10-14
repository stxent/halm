/*
 * spi.c
 * Copyright (C) 2018, 2023 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/platform/stm32/spi.h>
#include <halm/platform/stm32/spi_defs.h>
#include <assert.h>
#include <stdbool.h>
/*----------------------------------------------------------------------------*/
#define DUMMY_FRAME 0xFF
/*----------------------------------------------------------------------------*/
static void dmaHandler(void *);
static bool dmaSetup(struct Spi *, uint8_t, uint8_t);
static void dmaSetupRx(struct Dma *, struct Dma *);
static void dmaSetupRxTx(struct Dma *, struct Dma *);
static void dmaSetupTx(struct Dma *, struct Dma *);
static enum Result getStatus(const struct Spi *);
static size_t transferData(struct Spi *, const void *, void *, size_t);

#ifdef CONFIG_PLATFORM_STM32_SPI_PM
static void powerStateHandler(void *, enum PmState);
#endif
/*----------------------------------------------------------------------------*/
static enum Result spiInit(void *, const void *);
static void spiSetCallback(void *, void (*)(void *), void *);
static enum Result spiGetParam(void *, int, void *);
static enum Result spiSetParam(void *, int, const void *);
static size_t spiRead(void *, void *, size_t);
static size_t spiWrite(void *, const void *, size_t);

#ifndef CONFIG_PLATFORM_STM32_SPI_NO_DEINIT
static void spiDeinit(void *);
#else
#define spiDeinit deletedDestructorTrap
#endif
/*----------------------------------------------------------------------------*/
const struct InterfaceClass * const Spi = &(const struct InterfaceClass){
    .size = sizeof(struct Spi),
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
  struct Spi * const interface = object;

  if (interface->callback != NULL)
  {
    if (interface->invoked)
      interface->callback(interface->callbackArgument);
    else
      interface->invoked = true;
  }
}
/*----------------------------------------------------------------------------*/
static bool dmaSetup(struct Spi *interface, uint8_t rxStream, uint8_t txStream)
{
  interface->rxDma = spiMakeOneShotDma(
      interface->base.channel,
      rxStream,
      DMA_PRIORITY_LOW,
      DMA_TYPE_P2M
  );
  if (interface->rxDma == NULL)
    return false;

  interface->txDma = spiMakeOneShotDma(
      interface->base.channel,
      txStream,
      DMA_PRIORITY_LOW,
      DMA_TYPE_M2P
  );
  if (interface->txDma == NULL)
    return false;

  return true;
}
/*----------------------------------------------------------------------------*/
static void dmaSetupRx(struct Dma *rx, struct Dma *tx)
{
  static const struct DmaSettings dmaSettings[] = {
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
              .burst = DMA_BURST_1,
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
  static const struct DmaSettings dmaSettings[] = {
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
  static const struct DmaSettings dmaSettings[] = {
      {
          .source = {
              .burst = DMA_BURST_1,
              .width = DMA_WIDTH_BYTE,
              .increment = false
          },
          .destination = {
              .burst = DMA_BURST_1,
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
static enum Result getStatus(const struct Spi *interface)
{
  STM_SPI_Type * const reg = interface->base.reg;
  enum Result res;

  if (reg->SR & SR_BSY)
    res = E_BUSY;
  else
    res = dmaStatus(interface->rxDma);

  return res;
}
/*----------------------------------------------------------------------------*/
#ifdef CONFIG_PLATFORM_STM32_SPI_PM
static void powerStateHandler(void *object, enum PmState state)
{
  if (state == PM_ACTIVE)
  {
    struct Spi * const interface = object;
    spiSetRate(&interface->base, interface->rate);
  }
}
#endif
/*----------------------------------------------------------------------------*/
static size_t transferData(struct Spi *interface, const void *txSource,
    void *rxSink, size_t length)
{
  STM_SPI_Type * const reg = interface->base.reg;

  interface->invoked = false;
  interface->sink = NULL;
  dmaAppend(interface->rxDma, rxSink, (const void *)&reg->DR, length);
  dmaAppend(interface->txDma, (void *)&reg->DR, txSource, length);

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
  const struct SpiConfig * const config = configBase;
  assert(config != NULL);

  const struct SpiBaseConfig baseConfig = {
      .cs = 0,
      .miso = config->miso,
      .mosi = config->mosi,
      .sck = config->sck,
      .channel = config->channel,
      .slave = false
  };
  struct Spi * const interface = object;
  enum Result res;

  /* Call base class constructor */
  if ((res = SpiBase->init(interface, &baseConfig)) != E_OK)
    return res;

  if (!dmaSetup(interface, config->rxDma, config->txDma))
    return E_ERROR;

  interface->callback = NULL;
  interface->rate = config->rate;
  interface->sink = NULL;
  interface->blocking = true;
  interface->unidir = true;

  STM_SPI_Type * const reg = interface->base.reg;

  /* Set mode of the interface */
  uint32_t controlValue = CR1_MSTR;

  if (config->mode & 0x01)
    controlValue |= CR1_CPHA;
  if (config->mode & 0x02)
    controlValue |= CR1_CPOL;
  reg->CR1 = controlValue;

  /* Set desired baud rate */
  spiSetRate(object, interface->rate);

#ifdef CONFIG_PLATFORM_STM32_SPI_PM
  if ((res = pmRegister(powerStateHandler, interface)) != E_OK)
    return res;
#endif

  /* Enable the peripheral */
  reg->CR2 = CR2_RXDMAEN | CR2_TXDMAEN | CR2_SSOE;
  reg->CR1 |= CR1_SPE;

  return E_OK;
}
/*----------------------------------------------------------------------------*/
#ifndef CONFIG_PLATFORM_STM32_SPI_NO_DEINIT
static void spiDeinit(void *object)
{
  struct Spi * const interface = object;
  STM_SPI_Type * const reg = interface->base.reg;

  /* Disable the peripheral */
  reg->CR1 = 0;

#ifdef CONFIG_PLATFORM_STM32_SPI_PM
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
  struct Spi * const interface = object;

  interface->callbackArgument = argument;
  interface->callback = callback;
}
/*----------------------------------------------------------------------------*/
static enum Result spiGetParam(void *object, int parameter, void *data)
{
  struct Spi * const interface = object;

#ifndef CONFIG_PLATFORM_STM32_SPI_RC
  (void)data;
#endif

  switch ((enum IfParameter)parameter)
  {
#ifdef CONFIG_PLATFORM_STM32_SPI_RC
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
  struct Spi * const interface = object;

#ifndef CONFIG_PLATFORM_STM32_SPI_RC
  (void)data;
#endif

  switch ((enum SPIParameter)parameter)
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
      dmaSetCallback(interface->rxDma, NULL, NULL);
      dmaSetCallback(interface->txDma, NULL, NULL);
      interface->blocking = true;
      return E_OK;

#ifdef CONFIG_PLATFORM_STM32_SPI_RC
    case IF_RATE:
      interface->rate = *(const uint32_t *)data;
      spiSetRate(object, interface->rate);
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

  struct Spi * const interface = object;

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

  struct Spi * const interface = object;

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
