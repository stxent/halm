/*
 * spi.c
 * Copyright (C) 2013 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <assert.h>
#include <xcore/memory.h>
#include <halm/platform/nxp/spi.h>
#include <halm/platform/nxp/ssp_defs.h>
#include <halm/pm.h>
/*----------------------------------------------------------------------------*/
#define DUMMY_FRAME 0xFF
#define FIFO_DEPTH  8
/*----------------------------------------------------------------------------*/
static void interruptHandler(void *);
static size_t transferData(struct Spi *, size_t);

#ifdef CONFIG_PLATFORM_NXP_SSP_PM
static void powerStateHandler(void *, enum PmState);
#endif
/*----------------------------------------------------------------------------*/
static enum Result spiInit(void *, const void *);
static enum Result spiSetCallback(void *, void (*)(void *), void *);
static enum Result spiGetParam(void *, enum IfParameter, void *);
static enum Result spiSetParam(void *, enum IfParameter, const void *);
static size_t spiRead(void *, void *, size_t);
static size_t spiWrite(void *, const void *, size_t);

#ifndef CONFIG_PLATFORM_NXP_SSP_NO_DEINIT
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
  LPC_SSP_Type * const reg = interface->base.reg;
  size_t received = 0;

  if (reg->RIS & RIS_RXRIS)
  {
    /* FIFO is at least half full */
    if (interface->rxBuffer)
    {
      for (size_t i = 0; i < FIFO_DEPTH / 2; ++i)
        *interface->rxBuffer++ = reg->DR;
    }
    else
    {
      for (size_t i = 0; i < FIFO_DEPTH / 2; ++i)
        (void)reg->DR;
    }

    received += FIFO_DEPTH / 2;
  }

  if (interface->rxBuffer)
  {
    while (reg->SR & SR_RNE)
    {
      *interface->rxBuffer++ = reg->DR;
      ++received;
    }
  }
  else
  {
    while (reg->SR & SR_RNE)
    {
      (void)reg->DR;
      ++received;
    }
  }

  interface->rxLeft -= received;

  const size_t space = FIFO_DEPTH - (interface->rxLeft - interface->txLeft);
  size_t pending = MIN(space, interface->txLeft);

  interface->txLeft -= pending;

  if (interface->txBuffer)
  {
    while (pending--)
      reg->DR = *interface->txBuffer++;
  }
  else
  {
    while (pending--)
      reg->DR = DUMMY_FRAME;
  }

  if (!interface->rxLeft)
  {
    /* Disable interrupts */
    reg->IMSC = 0;

    /*
     * Reset the pointer to an input buffer only. The pointer for
     * an output buffer will be reinitialized in read and write functions.
     */
    interface->rxBuffer = 0;

    if (interface->callback)
      interface->callback(interface->callbackArgument);
  }
}
/*----------------------------------------------------------------------------*/
#ifdef CONFIG_PLATFORM_NXP_SSP_PM
static void powerStateHandler(void *object, enum PmState state)
{
  struct Spi * const interface = object;

  if (state == PM_ACTIVE)
    sspSetRate(object, interface->rate);
}
#endif
/*----------------------------------------------------------------------------*/
static size_t transferData(struct Spi *interface, size_t length)
{
  LPC_SSP_Type * const reg = interface->base.reg;

  interface->rxLeft = interface->txLeft = length;

  /* Clear interrupt flags and enable interrupts */
  reg->ICR = ICR_RORIC | ICR_RTIC;
  reg->IMSC = IMSC_RXIM | IMSC_RTIM;

  /* Initiate transmission by setting pending interrupt flag */
  irqSetPending(interface->base.irq);

  if (interface->blocking)
  {
    while (interface->rxLeft || reg->SR & SR_BSY)
      barrier();
  }

  return length;
}
/*----------------------------------------------------------------------------*/
static enum Result spiInit(void *object, const void *configBase)
{
  const struct SpiConfig * const config = configBase;
  assert(config);

  const struct SspBaseConfig baseConfig = {
      .channel = config->channel,
      .miso = config->miso,
      .mosi = config->mosi,
      .sck = config->sck,
      .cs = 0
  };
  struct Spi * const interface = object;
  enum Result res;

  /* Call base class constructor */
  if ((res = SspBase->init(object, &baseConfig)) != E_OK)
    return res;

  interface->base.handler = interruptHandler;
  interface->callback = 0;
  interface->rate = config->rate;
  interface->rxBuffer = 0;
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

#ifdef CONFIG_PLATFORM_NXP_SSP_PM
  if ((res = pmRegister(powerStateHandler, interface)) != E_OK)
    return res;
#endif

  /* Enable the peripheral */
  reg->CR1 = CR1_SSE;

  irqSetPriority(interface->base.irq, config->priority);
  irqEnable(interface->base.irq);

  return E_OK;
}
/*----------------------------------------------------------------------------*/
#ifndef CONFIG_PLATFORM_NXP_SSP_NO_DEINIT
static void spiDeinit(void *object)
{
  struct Spi * const interface = object;
  LPC_SSP_Type * const reg = interface->base.reg;

  /* Disable the peripheral */
  irqDisable(interface->base.irq);
  reg->CR1 = 0;

#ifdef CONFIG_PLATFORM_NXP_SSP_PM
  pmUnregister(interface);
#endif

  SspBase->deinit(interface);
}
#endif
/*----------------------------------------------------------------------------*/
static enum Result spiSetCallback(void *object, void (*callback)(void *),
    void *argument)
{
  struct Spi * const interface = object;

  interface->callbackArgument = argument;
  interface->callback = callback;
  return E_OK;
}
/*----------------------------------------------------------------------------*/
static enum Result spiGetParam(void *object, enum IfParameter parameter,
    void *data)
{
  struct Spi * const interface = object;

#ifndef CONFIG_PLATFORM_NXP_SSP_RC
  (void)data;
#endif

  switch (parameter)
  {
#ifdef CONFIG_PLATFORM_NXP_SSP_RC
    case IF_RATE:
      *(uint32_t *)data = sspGetRate(object);
      return E_OK;
#endif

    case IF_STATUS:
    {
      const LPC_SSP_Type * const reg = interface->base.reg;
      return (interface->rxLeft || reg->SR & SR_BSY) ? E_BUSY : E_OK;
    }

    default:
      return E_INVALID;
  }
}
/*----------------------------------------------------------------------------*/
static enum Result spiSetParam(void *object, enum IfParameter parameter,
    const void *data)
{
  struct Spi * const interface = object;

#ifndef CONFIG_PLATFORM_NXP_SSP_RC
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

  switch (parameter)
  {
    case IF_BLOCKING:
      interface->blocking = true;
      return E_OK;

#ifdef CONFIG_PLATFORM_NXP_SSP_RC
    case IF_RATE:
      interface->rate = *(const uint32_t *)data;
      sspSetRate(object, interface->rate);
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
    interface->txBuffer = 0;
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
