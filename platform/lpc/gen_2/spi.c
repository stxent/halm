/*
 * spi.c
 * Copyright (C) 2025 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/platform/lpc/spi.h>
#include <halm/platform/lpc/spi_defs.h>
#include <halm/pm.h>
#include <xcore/memory.h>
#include <assert.h>
/*----------------------------------------------------------------------------*/
#define DUMMY_FRAME 0xFF
#define FIFO_DEPTH  8
/*----------------------------------------------------------------------------*/
static void interruptHandler(void *);
static size_t transferData(struct Spi *, size_t);

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
  LPC_SPI_Type * const reg = interface->base.reg;
  const uint32_t status = reg->INTSTAT;
  bool event = false;

  /* Handle reception */
  if (status & INTSTAT_RXRDY)
  {
    *interface->rxBuffer++ = reg->RXDAT;

    if (!--interface->rxLeft)
    {
      reg->INTENCLR = INTENCLR_RXRDYEN;
      event = true;
    }
  }

  /* Handle transmission ready */
  if (status & INTSTAT_TXRDY)
  {
    if (interface->txLeft)
    {
      if (interface->txBuffer != NULL)
        reg->TXDAT = *interface->txBuffer++;
      else
        reg->TXDAT = DUMMY_FRAME;

      --interface->txLeft;
    }
    else
    {
      reg->INTENCLR = INTENCLR_TXRDYEN;

      if (interface->rxBuffer == NULL)
        event = true;
    }
  }

  if (event)
  {
    reg->STAT = STAT_SSA | STAT_SSD | STAT_ENDTRANSFER;

    interface->rxBuffer = NULL;
    interface->txBuffer = NULL;

    if (interface->callback != NULL)
      interface->callback(interface->callbackArgument);
  }
}
/*----------------------------------------------------------------------------*/
#ifdef CONFIG_PLATFORM_LPC_SPI_PM
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
  LPC_SPI_Type * const reg = interface->base.reg;

  interface->txLeft = length;
  if (interface->rxBuffer != NULL)
  {
    interface->rxLeft = length;
    reg->TXCTL &= ~TXCTL_RXIGNORE;

    /* Initiate transmission by enabling RX and TX ready interrupts */
    reg->INTENSET = INTENSET_RXRDYEN | INTENSET_TXRDYEN;
  }
  else
  {
    interface->rxLeft = 0;
    reg->TXCTL |= TXCTL_RXIGNORE;

    /* Initiate transmission by enabling TX ready interrupt */
    reg->INTENSET = INTENSET_TXRDYEN;
  }

  if (interface->blocking)
  {
    while (interface->rxBuffer != NULL || interface->txBuffer != NULL)
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
      .channel = config->channel
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

  irqSetPriority(interface->base.irq, config->priority);
  irqEnable(interface->base.irq);

  return E_OK;
}
/*----------------------------------------------------------------------------*/
#ifndef CONFIG_PLATFORM_LPC_SPI_NO_DEINIT
static void spiDeinit(void *object)
{
  struct Spi * const interface = object;
  LPC_SPI_Type * const reg = interface->base.reg;

  /* Disable the peripheral */
  irqDisable(interface->base.irq);
  reg->CFG = 0;

#ifdef CONFIG_PLATFORM_LPC_SPI_PM
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
    {
      const LPC_SPI_Type * const reg = interface->base.reg;
      return (interface->rxLeft || !(reg->STAT & STAT_MSTIDLE)) ? E_BUSY : E_OK;
    }

    default:
      return E_INVALID;
  }
}
/*----------------------------------------------------------------------------*/
static enum Result spiSetParam(void *object, int parameter, const void *data)
{
  struct Spi * const interface = object;

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
