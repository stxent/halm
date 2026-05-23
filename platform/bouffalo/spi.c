/*
 * spi.c
 * Copyright (C) 2026 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/platform/bouffalo/spi.h>
#include <halm/platform/bouffalo/spi_defs.h>
#include <halm/pm.h>
#include <assert.h>
/*----------------------------------------------------------------------------*/
#define DUMMY_FRAME 0xFF
#define FIFO_DEPTH  4
/*----------------------------------------------------------------------------*/
static void interruptHandler(void *);
static size_t transferData(struct Spi *, size_t);

#ifdef CONFIG_PLATFORM_BOUFFALO_SPI_PM
static void powerStateHandler(void *, enum PmState);
#endif
/*----------------------------------------------------------------------------*/
static enum Result spiInit(void *, const void *);
static void spiSetCallback(void *, void (*)(void *), void *);
static enum Result spiGetParam(void *, int, void *);
static enum Result spiSetParam(void *, int, const void *);
static size_t spiRead(void *, void *, size_t);
static size_t spiWrite(void *, const void *, size_t);

#ifndef CONFIG_PLATFORM_BOUFFALO_SPI_NO_DEINIT
static void spiDeinit(void *);
#else
#  define spiDeinit deletedDestructorTrap
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
static void interruptHandler(void *object)
{
  struct Spi * const interface = object;
  BL_SPI_Type * const reg = interface->base.reg;

  /* Handle reception */
  const size_t received = FIFO_CONFIG1_RFCNT_VALUE(reg->FIFO_CONFIG1);

  if (interface->rxBuffer != NULL)
  {
    for (size_t index = 0; index < received; ++index)
      *interface->rxBuffer++ = reg->FIFO_RDATA;
  }
  else
  {
    for (size_t index = 0; index < received; ++index)
      (void)reg->FIFO_RDATA;
  }

  interface->rxLeft -= received;

  /* Handle transmission */
  if (interface->txLeft)
  {
    const uint32_t config = reg->FIFO_CONFIG1;
    const size_t rxSpace = FIFO_DEPTH - FIFO_CONFIG1_RFCNT_VALUE(config);
    const size_t txSpace = FIFO_CONFIG1_TFCNT_VALUE(config);
    size_t pending = MIN(MIN(rxSpace, txSpace), interface->txLeft);

    interface->txLeft -= pending;

    if (interface->txBuffer != NULL)
    {
      while (pending--)
        reg->FIFO_WDATA = *interface->txBuffer++;
    }
    else
    {
      while (pending--)
        reg->FIFO_WDATA = DUMMY_FRAME;
    }

    if (!interface->txLeft)
      reg->INT_STS |= INT_STS_TXFMASK;
  }

  /* Stop the transfer when all frames are received */
  if (!interface->rxLeft)
  {
    /*
     * Reset the pointer to an input buffer only. The pointer for
     * an output buffer will be reinitialized in read and write functions.
     */
    interface->rxBuffer = NULL;

    if (interface->callback != NULL)
      interface->callback(interface->callbackArgument);
  }
  else if (interface->rxLeft < FIFO_DEPTH / 2)
  {
    reg->FIFO_CONFIG1 = (reg->FIFO_CONFIG1 & ~FIFO_CONFIG1_RFTH_MASK)
        | FIFO_CONFIG1_RFTH(interface->rxLeft - 1);
  }
}
/*----------------------------------------------------------------------------*/
#ifdef CONFIG_PLATFORM_BOUFFALO_SPI_PM
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
static size_t transferData(struct Spi *interface, size_t length)
{
  BL_SPI_Type * const reg = interface->base.reg;
  const size_t chunk = MIN(length, FIFO_DEPTH / 2);

  interface->rxLeft = interface->txLeft = length;

  /* Clear FIFO, configure levels and enable interrupts */
  reg->FIFO_CONFIG0 = FIFO_CONFIG0_TFC | FIFO_CONFIG0_RFC;
  reg->FIFO_CONFIG1 = FIFO_CONFIG1_TFTH(chunk) | FIFO_CONFIG1_RFTH(chunk - 1);
  reg->INT_STS &= ~INT_STS_TXFMASK;

  if (interface->blocking)
  {
    while (interface->rxLeft || (reg->BUS_BUSY))
      barrier();
  }

  return length;
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
      .master = true
  };
  struct Spi * const interface = object;
  enum Result res;

  /* Call base class constructor */
  if ((res = SpiBase->init(interface, &baseConfig)) != E_OK)
    return res;

  interface->base.handler = interruptHandler;
  interface->callback = NULL;
  interface->rate = config->rate;
  interface->rxBuffer = NULL;
  interface->blocking = true;
  interface->unidir = true;

  BL_SPI_Type * const reg = interface->base.reg;

  /* Disable the controller before changing the configuration */
  reg->CONFIG = 0;

  /* Set data width */
  reg->CONFIG = CONFIG_FSIZE(FSIZE_8_BIT) | CONFIG_MCEN;
  /* Configure interrupts */
  reg->INT_STS = (INT_STS_MASK_ALL & ~INT_STS_RXFMASK)
      | (INT_STS_TXFEN | INT_STS_RXFEN);

  /* Set the desired data rate */
  if (!spiSetRate(&interface->base, interface->rate))
    return E_VALUE;
  /* Set SPI mode */
  spiSetMode(&interface->base, config->mode);

#ifdef CONFIG_PLATFORM_BOUFFALO_SPI_PM
  if ((res = pmRegister(powerStateHandler, interface)) != E_OK)
    return res;
#endif

  /* Enable the peripheral in master mode */
  reg->CONFIG |= CONFIG_MEN;

  irqSetPriority(interface->base.irq, config->priority);
  irqEnable(interface->base.irq);

  return E_OK;
}
/*----------------------------------------------------------------------------*/
#ifndef CONFIG_PLATFORM_BOUFFALO_SPI_NO_DEINIT
static void spiDeinit(void *object)
{
  struct Spi * const interface = object;
  BL_SPI_Type * const reg = interface->base.reg;

  /* Disable the peripheral */
  irqDisable(interface->base.irq);
  reg->CONFIG = 0;

#ifdef CONFIG_PLATFORM_BOUFFALO_SPI_PM
  pmUnregister(interface);
#endif

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

#ifndef CONFIG_PLATFORM_BOUFFALO_SPI_RC
  (void)data;
#endif

  switch ((enum SPIParameter)parameter)
  {
#ifdef CONFIG_PLATFORM_BOUFFALO_SPI_RC
    case IF_SPI_MODE:
      *(uint8_t *)data = spiGetMode(&interface->base);
      return E_OK;
#endif

    default:
      break;
  }

  switch ((enum IfParameter)parameter)
  {
#ifdef CONFIG_PLATFORM_BOUFFALO_SPI_RC
    case IF_RATE:
      *(uint32_t *)data = spiGetRate(object);
      return E_OK;
#endif

    case IF_STATUS:
    {
      const BL_SPI_Type * const reg = interface->base.reg;
      return (interface->rxLeft || reg->BUS_BUSY) ? E_BUSY : E_OK;
    }

    default:
      return E_INVALID;
  }
}
/*----------------------------------------------------------------------------*/
static enum Result spiSetParam(void *object, int parameter, const void *data)
{
  struct Spi * const interface = object;

#ifndef CONFIG_PLATFORM_BOUFFALO_SPI_RC
  (void)data;
#endif

  switch ((enum SPIParameter)parameter)
  {
#ifdef CONFIG_PLATFORM_BOUFFALO_SPI_RC
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

#ifdef CONFIG_PLATFORM_BOUFFALO_SPI_RC
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

  struct Spi * const interface = object;

  interface->rxBuffer = buffer;
  if (interface->unidir)
  {
    interface->txBuffer = NULL;
    return transferData(interface, length);
  }
  else
    return length;
}
/*----------------------------------------------------------------------------*/
static size_t spiWrite(void *object, const void *buffer, size_t length)
{
  if (!length)
    return 0;

  struct Spi * const interface = object;

  interface->txBuffer = buffer;
  return transferData(interface, length);
}
