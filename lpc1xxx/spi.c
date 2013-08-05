/*
 * spi.c
 * Copyright (C) 2013 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <assert.h>
#include "spi.h"
#include "ssp_defs.h"
/*----------------------------------------------------------------------------*/
#define DEFAULT_PRIORITY 255 /* Lowest interrupt priority in Cortex-M3 */
/*----------------------------------------------------------------------------*/
static void interruptHandler(void *);
/*----------------------------------------------------------------------------*/
static enum result spiInit(void *, const void *);
static void spiDeinit(void *);
static enum result spiCallback(void *, void (*)(void *), void *);
static enum result spiGet(void *, enum ifOption, void *);
static enum result spiSet(void *, enum ifOption, const void *);
static uint32_t spiRead(void *, uint8_t *, uint32_t);
static uint32_t spiWrite(void *, const uint8_t *, uint32_t);
/*----------------------------------------------------------------------------*/
static const struct InterfaceClass spiTable = {
    .size = sizeof(struct Spi),
    .init = spiInit,
    .deinit = spiDeinit,

    .callback = spiCallback,
    .get = spiGet,
    .set = spiSet,
    .read = spiRead,
    .write = spiWrite
};
/*----------------------------------------------------------------------------*/
const struct InterfaceClass *Spi = &spiTable;
/*----------------------------------------------------------------------------*/
static void interruptHandler(void *object)
{
  struct Spi *interface = object;
  uint8_t status = interface->parent.reg->MIS;

  if (status & (MIS_RXMIS | MIS_RTMIS))
  {
    /* Frame will be removed from FIFO after reading from DR register */
    while (interface->parent.reg->SR & SR_RNE && interface->left)
    {
      *interface->rxBuffer++ = interface->parent.reg->DR;
      --interface->left;
    }
    /* Fill transmit FIFO with dummy frames */
    while (interface->parent.reg->SR & SR_TNF && interface->fill)
    {
      interface->parent.reg->DR = 0xFF; /* TODO Select dummy frame value */
      --interface->fill;
    }
    /* Disable receive interrupts when all frames have been received */
    if (!interface->left)
    {
      interface->parent.reg->IMSC &= ~(IMSC_RXIM | IMSC_RTIM);
      if (interface->callback)
        interface->callback(interface->callbackArgument);
    }
  }
  if (status & MIS_TXMIS)
  {
    /* Fill transmit FIFO */
    while (interface->parent.reg->SR & SR_TNF && interface->left)
    {
      interface->parent.reg->DR = *interface->txBuffer++;
      --interface->left;
    }
    /* Disable transmit interrupt when all frames have been pushed to FIFO */
    if (!interface->left)
    {
      interface->parent.reg->IMSC &= ~IMSC_TXIM;
      if (interface->callback)
        interface->callback(interface->callbackArgument);
    }
  }
}
/*----------------------------------------------------------------------------*/
static enum result spiInit(void *object, const void *configPtr)
{
  const struct SpiConfig * const config = configPtr;
  struct Spi *interface = object;
  struct SspConfig parentConfig = {
      .frame = 8 /* Fixed frame size */
  };
  enum result res;

  /* Check interface configuration data */
  assert(config);

  /* Initialize parent configuration structure */
  parentConfig.channel = config->channel;
  parentConfig.sck = config->sck;
  parentConfig.miso = config->miso;
  parentConfig.mosi = config->mosi;
  parentConfig.rate = config->rate;
  parentConfig.cs = config->cs;

  /* Call SSP class constructor */
  if ((res = Ssp->init(object, &parentConfig)) != E_OK)
    return res;
  interface->channelLock = MUTEX_UNLOCKED;
  interface->callback = 0;
  interface->blocking = true;

  /* Set pointer to hardware interrupt handler */
  interface->parent.handler = interruptHandler;

  /* Set interrupt priority, lowest by default */
  nvicSetPriority(interface->parent.irq, DEFAULT_PRIORITY);
  /* Enable UART interrupt */
  nvicEnable(interface->parent.irq);
  return E_OK;
}
/*----------------------------------------------------------------------------*/
static void spiDeinit(void *object)
{
  struct Spi *interface = object;

  nvicDisable(interface->parent.irq);
  Ssp->deinit(interface); /* Call SSP class destructor */
}
/*----------------------------------------------------------------------------*/
static enum result spiCallback(void *object, void (*callback)(void *),
    void *argument)
{
  struct Spi *interface = object;

  interface->callback = callback;
  interface->callbackArgument = argument;
  return E_OK;
}
/*----------------------------------------------------------------------------*/
static enum result spiGet(void *object, enum ifOption option, void *data)
{
  struct Spi *interface = object;

  switch (option)
  {
    case IF_BUSY:
      *(uint32_t *)data = interface->left
          || (interface->parent.reg->SR & SR_BSY);
      return E_OK;
    case IF_NONBLOCKING:
      *(uint32_t *)data = !interface->blocking;
      return E_OK;
    case IF_PRIORITY:
      *(uint32_t *)data = nvicGetPriority(interface->parent.irq);
      return E_OK;
    case IF_RATE:
      *(uint32_t *)data = sspGetRate(object);
      return E_OK;
    default:
      return E_ERROR;
  }
}
/*----------------------------------------------------------------------------*/
static enum result spiSet(void *object, enum ifOption option, const void *data)
{
  struct Spi *interface = object;

  switch (option)
  {
    case IF_LOCK:
      if (*(uint32_t *)data)
      {
        if (interface->blocking)
          return mutexTryLock(&interface->channelLock) ? E_OK : E_BUSY;
        else
          mutexLock(&interface->channelLock);
      }
      else
      {
        mutexUnlock(&interface->channelLock);
      }
      return E_OK;
    case IF_NONBLOCKING:
      interface->blocking = *(uint32_t *)data ? false : true;
      return E_OK;
    case IF_PRIORITY:
      nvicSetPriority(interface->parent.irq, *(uint32_t *)data);
      return E_OK;
    case IF_RATE:
      sspSetRate(object, *(uint32_t *)data);
      return E_OK;
    default:
      return E_ERROR;
  }
}
/*----------------------------------------------------------------------------*/
static uint32_t spiRead(void *object, uint8_t *buffer, uint32_t length)
{
  struct Spi *interface = object;

  /* Break all previous transactions */
  interface->parent.reg->IMSC &= ~(IMSC_TXIM | IMSC_RXIM | IMSC_RTIM);

  interface->fill = length;
  /* Pop all previously received frames */
  while (interface->parent.reg->SR & SR_RNE)
    (void)interface->parent.reg->DR;

  /* Fill transmit FIFO with dummy frames */
  while (interface->parent.reg->SR & SR_TNF && interface->fill)
  {
    interface->parent.reg->DR = 0xFF;
    --interface->fill;
  }

  interface->rxBuffer = buffer;
  interface->left = length;
  /* Enable receive FIFO half full and receive FIFO timeout interrupts */
  interface->parent.reg->IMSC |= IMSC_RXIM | IMSC_RTIM;

  if (interface->blocking)
    while (interface->left || (interface->parent.reg->SR & SR_BSY));

  return length;
}
/*----------------------------------------------------------------------------*/
static uint32_t spiWrite(void *object, const uint8_t *buffer, uint32_t length)
{
  struct Spi *interface = object;

  /* Break all previous transactions */
  interface->parent.reg->IMSC &= ~(IMSC_TXIM | IMSC_RXIM | IMSC_RTIM);

  interface->left = length;
  /* Fill transmit FIFO */
  while (interface->parent.reg->SR & SR_TNF && interface->left)
  {
    interface->parent.reg->DR = *buffer++;
    interface->left--;
  }

  /* Enable transmit interrupt when packet size is greater than FIFO length */
  if (interface->left)
  {
    interface->txBuffer = buffer;
    interface->parent.reg->IMSC |= IMSC_TXIM;
  }

  if (interface->blocking)
    while (interface->left || (interface->parent.reg->SR & SR_BSY));

  return length;
}
