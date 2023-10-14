/*
 * spi.c
 * Copyright (C) 2023 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/platform/numicro/spi.h>
#include <halm/platform/numicro/spi_defs.h>
#include <halm/pm.h>
#include <assert.h>
/*----------------------------------------------------------------------------*/
#define DUMMY_FRAME 0xFF
#define FIFO_DEPTH  8
/*----------------------------------------------------------------------------*/
static void interruptHandler(void *);
static size_t transferData(struct Spi *, size_t);

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
static void interruptHandler(void *object)
{
  struct Spi * const interface = object;
  NM_SPI_Type * const reg = interface->base.reg;

  /* Clear pending interrupt flags */
  reg->STATUS = STATUS_UNITIF | STATUS_RXOVIF | STATUS_RXTOIF;

  /* Handle reception */
  const uint32_t status = reg->STATUS;
  const size_t received = STATUS_RXCNT_VALUE(status);

  if (interface->rxBuffer != NULL)
  {
    for (size_t index = 0; index < received; ++index)
      *interface->rxBuffer++ = reg->RX;
  }
  else
  {
    for (size_t index = 0; index < received; ++index)
      (void)reg->RX;
  }

  interface->rxLeft -= received;

  /* Handle transmission */
  if (interface->txLeft)
  {
    const size_t space = FIFO_DEPTH - STATUS_TXCNT_VALUE(status);
    size_t pending = MIN(space, interface->txLeft);

    interface->txLeft -= pending;

    if (interface->txBuffer != NULL)
    {
      while (pending--)
        reg->TX = *interface->txBuffer++;
    }
    else
    {
      while (pending--)
        reg->TX = DUMMY_FRAME;
    }

    if (!interface->txLeft)
      reg->FIFOCTL &= ~FIFOCTL_TXTHIEN;
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
  else if (interface->rxLeft && interface->rxLeft < FIFO_DEPTH / 2)
  {
    reg->FIFOCTL = (reg->FIFOCTL & ~FIFOCTL_RXTH_MASK)
        | FIFOCTL_RXTH(interface->rxLeft);
  }
}
/*----------------------------------------------------------------------------*/
#ifdef CONFIG_PLATFORM_NUMICRO_SPI_PM
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
  NM_SPI_Type * const reg = interface->base.reg;
  const size_t chunk = MIN(length, FIFO_DEPTH / 2);

  interface->rxLeft = interface->txLeft = length;

  /* Clear FIFO, configure levels and enable interrupts */
  reg->FIFOCTL = FIFOCTL_TXTHIEN | FIFOCTL_TXTH(FIFO_DEPTH / 2)
      | FIFOCTL_RXTHIEN | FIFOCTL_RXTH(chunk);

  if (interface->blocking)
  {
    while (interface->rxLeft || (reg->STATUS & STATUS_BUSY))
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

  irqSetPriority(interface->base.irq, config->priority);
  irqEnable(interface->base.irq);

  return E_OK;
}
/*----------------------------------------------------------------------------*/
#ifndef CONFIG_PLATFORM_NUMICRO_SPI_NO_DEINIT
static void spiDeinit(void *object)
{
  struct Spi * const interface = object;
  NM_SPI_Type * const reg = interface->base.reg;

  /* Disable the peripheral */
  irqDisable(interface->base.irq);
  reg->CTL &= ~CTL_SPIEN;

#ifdef CONFIG_PLATFORM_NUMICRO_SPI_PM
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
    {
      const NM_SPI_Type * const reg = interface->base.reg;
      return (interface->rxLeft || reg->STATUS & STATUS_BUSY) ? E_BUSY : E_OK;
    }

    default:
      return E_INVALID;
  }
}
/*----------------------------------------------------------------------------*/
static enum Result spiSetParam(void *object, int parameter, const void *data)
{
  struct Spi * const interface = object;

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
