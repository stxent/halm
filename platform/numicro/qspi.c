/*
 * qspi.c
 * Copyright (C) 2023 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/platform/numicro/pdma_oneshot.h>
#include <halm/platform/numicro/qspi_defs.h>
#include <halm/platform/numicro/qspi.h>
#include <halm/pm.h>
#include <assert.h>
/*----------------------------------------------------------------------------*/
#define DUMMY_FRAME 0xFF

enum
{
  MODE_SERIAL,
  MODE_DUAL,
  MODE_QUAD
};
/*----------------------------------------------------------------------------*/
static void dmaHandler(void *);
static bool dmaSetup(struct Qspi *, uint8_t, uint8_t);
static void dmaSetupRx(struct Dma *, struct Dma *);
static void dmaSetupTx(struct Dma *, struct Dma *);
static enum Result getStatus(const struct Qspi *);
static void setInterfaceMode(struct Qspi *, uint8_t);
static size_t transferData(struct Qspi *, const void *, void *, size_t);

#ifdef CONFIG_PLATFORM_NUMICRO_QSPI_DDR
static void setDoubleDataRate(struct Qspi *, bool);
#endif
#ifdef CONFIG_PLATFORM_NUMICRO_QSPI_PM
static void powerStateHandler(void *, enum PmState);
#endif
/*----------------------------------------------------------------------------*/
static enum Result qspiInit(void *, const void *);
static void qspiSetCallback(void *, void (*)(void *), void *);
static enum Result qspiGetParam(void *, int, void *);
static enum Result qspiSetParam(void *, int, const void *);
static size_t qspiRead(void *, void *, size_t);
static size_t qspiWrite(void *, const void *, size_t);

#ifndef CONFIG_PLATFORM_NUMICRO_QSPI_NO_DEINIT
static void qspiDeinit(void *);
#else
#define qspiDeinit deletedDestructorTrap
#endif
/*----------------------------------------------------------------------------*/
const struct InterfaceClass * const Qspi = &(const struct InterfaceClass){
    .size = sizeof(struct Qspi),
    .init = qspiInit,
    .deinit = qspiDeinit,

    .setCallback = qspiSetCallback,
    .getParam = qspiGetParam,
    .setParam = qspiSetParam,
    .read = qspiRead,
    .write = qspiWrite
};
/*----------------------------------------------------------------------------*/
static void dmaHandler(void *object)
{
  struct Qspi * const interface = object;
  NM_QSPI_Type * const reg = interface->base.reg;

  if (interface->invoked)
  {
    reg->PDMACTL = 0;

    if (interface->callback)
      interface->callback(interface->callbackArgument);
  }
  else
    interface->invoked = true;
}
/*----------------------------------------------------------------------------*/
static bool dmaSetup(struct Qspi *interface, uint8_t rxChannel,
    uint8_t txChannel)
{
  const struct PdmaOneShotConfig dmaConfigs[] = {
      {
          .event = pdmaGetEventQspiRx(interface->base.channel),
          .channel = rxChannel
      }, {
          .event = pdmaGetEventQspiTx(interface->base.channel),
          .channel = txChannel
      }
  };

  interface->rxDma = init(PdmaOneShot, &dmaConfigs[0]);
  if (!interface->rxDma)
    return false;

  interface->txDma = init(PdmaOneShot, &dmaConfigs[1]);
  if (!interface->txDma)
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
static enum Result getStatus(const struct Qspi *interface)
{
  NM_QSPI_Type * const reg = interface->base.reg;
  enum Result res;

  if (reg->STATUS & STATUS_BUSY)
    res = E_BUSY;
  else
    res = dmaStatus(interface->rxDma);

  return res;
}
/*----------------------------------------------------------------------------*/
#ifdef CONFIG_PLATFORM_NUMICRO_QSPI_DDR
static void setDoubleDataRate(struct Qspi *interface, bool enable)
{
  NM_QSPI_Type * const reg = interface->base.reg;

  if (enable)
    reg->CTL |= CTL_TXDTR_EN;
  else
    reg->CTL &= ~CTL_TXDTR_EN;
}
#endif
/*----------------------------------------------------------------------------*/
static void setInterfaceMode(struct Qspi *interface, uint8_t width)
{
  NM_QSPI_Type * const reg = interface->base.reg;

  switch (width)
  {
    case MODE_DUAL:
      reg->CTL = (reg->CTL & ~CTL_QUADIOEN) | CTL_DUALIOEN;
      break;

    case MODE_QUAD:
      reg->CTL = (reg->CTL & ~CTL_DUALIOEN) | CTL_QUADIOEN;
      break;

    default:
      reg->CTL &= ~(CTL_DUALIOEN | CTL_QUADIOEN);
      break;
  }
}
/*----------------------------------------------------------------------------*/
#ifdef CONFIG_PLATFORM_NUMICRO_QSPI_PM
static void powerStateHandler(void *object, enum PmState state)
{
  struct Qspi * const interface = object;

  if (state == PM_ACTIVE)
    qspiSetRate(object, interface->rate);
}
#endif
/*----------------------------------------------------------------------------*/
static size_t transferData(struct Qspi *interface, const void *txSource,
    void *rxSink, size_t length)
{
  NM_QSPI_Type * const reg = interface->base.reg;

  interface->invoked = false;

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
static enum Result qspiInit(void *object, const void *configBase)
{
  const struct QspiConfig * const config = configBase;
  assert(config);

  const struct QspiBaseConfig baseConfig = {
      .cs = 0,
      .io0 = config->io0,
      .io1 = config->io1,
      .io2 = config->io2,
      .io3 = config->io3,
      .sck = config->sck,
      .channel = config->channel
  };
  struct Qspi * const interface = object;
  enum Result res;

  /* Call base class constructor */
  if ((res = QspiBase->init(interface, &baseConfig)) != E_OK)
    return res;

  if (!dmaSetup(interface, config->dma[0], config->dma[1]))
    return E_ERROR;

  interface->callback = 0;
  interface->rate = config->rate;
  interface->blocking = true;

  NM_QSPI_Type * const reg = interface->base.reg;

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
  qspiSetRate(&interface->base, interface->rate);
  /* Set SPI mode */
  qspiSetMode(&interface->base, config->mode);

  /* Reset PDMA control register */
  reg->PDMACTL = 0;

#ifdef CONFIG_PLATFORM_NUMICRO_QSPI_PM
  if ((res = pmRegister(powerStateHandler, interface)) != E_OK)
    return res;
#endif

  /* Enable the peripheral */
  reg->CTL |= CTL_SPIEN;

  return E_OK;
}
/*----------------------------------------------------------------------------*/
#ifndef CONFIG_PLATFORM_NUMICRO_QSPI_NO_DEINIT
static void qspiDeinit(void *object)
{
  struct Qspi * const interface = object;
  NM_QSPI_Type * const reg = interface->base.reg;

  /* Disable the peripheral */
  reg->CTL &= ~CTL_SPIEN;

#ifdef CONFIG_PLATFORM_NUMICRO_QSPI_PM
  pmUnregister(interface);
#endif

  deinit(interface->txDma);
  deinit(interface->rxDma);

  QspiBase->deinit(interface);
}
#endif
/*----------------------------------------------------------------------------*/
static void qspiSetCallback(void *object, void (*callback)(void *),
    void *argument)
{
  struct Qspi * const interface = object;

  interface->callbackArgument = argument;
  interface->callback = callback;
}
/*----------------------------------------------------------------------------*/
static enum Result qspiGetParam(void *object, int parameter, void *data)
{
  struct Qspi * const interface = object;

#ifndef CONFIG_PLATFORM_NUMICRO_QSPI_RC
  (void)data;
#endif

  switch ((enum QSPIParameter)parameter)
  {
#ifdef CONFIG_PLATFORM_NUMICRO_QSPI_RC
    case IF_QSPI_MODE:
      *(uint8_t *)data = qspiGetMode(&interface->base);
      return E_OK;
#endif

    default:
      break;
  }

  switch ((enum IfParameter)parameter)
  {
#ifdef CONFIG_PLATFORM_NUMICRO_QSPI_RC
    case IF_RATE:
      *(uint32_t *)data = qspiGetRate(object);
      return E_OK;
#endif

    case IF_STATUS:
      return getStatus(interface);

    default:
      return E_INVALID;
  }
}
/*----------------------------------------------------------------------------*/
static enum Result qspiSetParam(void *object, int parameter, const void *data)
{
  struct Qspi * const interface = object;

#ifndef CONFIG_PLATFORM_NUMICRO_QSPI_RC
  (void)data;
#endif

  switch ((enum QSPIParameter)parameter)
  {
    case IF_QSPI_MODE:
      qspiSetMode(&interface->base, *(const uint8_t *)data);
      return E_OK;

    case IF_QSPI_SERIAL:
      setInterfaceMode(interface, MODE_SERIAL);
      return E_OK;

    case IF_QSPI_DUAL:
      setInterfaceMode(interface, MODE_DUAL);
      return E_OK;

    case IF_QSPI_QUAD:
      setInterfaceMode(interface, MODE_QUAD);
      return E_OK;

#ifdef CONFIG_PLATFORM_NUMICRO_QSPI_DDR
    case IF_QSPI_SDR:
      setDoubleDataRate(interface, false);
      return E_OK;

    case IF_QSPI_DDR:
      setDoubleDataRate(interface, true);
      return E_OK;
#endif

    default:
      break;
  }

  switch ((enum IfParameter)parameter)
  {
    case IF_BLOCKING:
      interface->blocking = true;
      return E_OK;

#ifdef CONFIG_PLATFORM_NUMICRO_QSPI_RC
    case IF_RATE:
      interface->rate = *(const uint32_t *)data;
      qspiSetRate(&interface->base, interface->rate);
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
static size_t qspiRead(void *object, void *buffer, size_t length)
{
  if (!length)
    return 0;

  struct Qspi * const interface = object;
  NM_QSPI_Type * const reg = interface->base.reg;

  /* Input direction */
  reg->CTL &= ~CTL_DATDIR;

  interface->dummy = DUMMY_FRAME;
  dmaSetupRx(interface->rxDma, interface->txDma);
  return transferData(interface, &interface->dummy, buffer, length);
}
/*----------------------------------------------------------------------------*/
static size_t qspiWrite(void *object, const void *buffer, size_t length)
{
  if (!length)
    return 0;

  struct Qspi * const interface = object;
  NM_QSPI_Type * const reg = interface->base.reg;

  /* Output direction */
  reg->CTL |= CTL_DATDIR;

  dmaSetupTx(interface->rxDma, interface->txDma);
  return transferData(interface, buffer, &interface->dummy, length);
}
