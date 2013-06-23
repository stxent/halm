/*
 * spi.c
 * Copyright (C) 2013 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <assert.h>
#include "spi.h"
#include "ssp_defs.h"
/*----------------------------------------------------------------------------*/
#define DEFAULT_PRIORITY 15 /* Lowest interrupt priority in Cortex-M3 */
/*----------------------------------------------------------------------------*/
static void interruptHandler(void *);
/*----------------------------------------------------------------------------*/
static enum result spiInit(void *, const void *);
static void spiDeinit(void *);
static uint32_t spiRead(void *, uint8_t *, uint32_t);
static uint32_t spiWrite(void *, const uint8_t *, uint32_t);
static enum result spiGet(void *, enum ifOption, void *);
static enum result spiSet(void *, enum ifOption, const void *);
/*----------------------------------------------------------------------------*/
static const struct InterfaceClass spiTable = {
    .size = sizeof(struct Spi),
    .init = spiInit,
    .deinit = spiDeinit,

    .read = spiRead,
    .write = spiWrite,
    .get = spiGet,
    .set = spiSet
};
/*----------------------------------------------------------------------------*/
const struct InterfaceClass *Spi = &spiTable;
/*----------------------------------------------------------------------------*/
static void interruptHandler(void *object)
{
  struct Spi *device = object;
  uint8_t status = device->parent.reg->MIS;

  if (status & (MIS_RXMIS | MIS_RTMIS))
  {
    /* Frame will be removed from FIFO after reading from DR register */
    while (device->parent.reg->SR & SR_RNE && device->left)
    {
      *device->rxBuffer++ = device->parent.reg->DR;
      --device->left;
    }
    /* Fill transmit FIFO with dummy frames */
    /* TODO Move to TX interrupt? */
    while (device->parent.reg->SR & SR_TNF && device->fill)
    {
      device->parent.reg->DR = 0xFF; /* TODO Select dummy frame value */
      --device->fill;
    }
    /* Disable receive interrupts when all frames have been received */
    if (!device->left)
      device->parent.reg->IMSC &= ~(IMSC_RXIM | IMSC_RTIM);
  }
  if (status & MIS_TXMIS)
  {
    /* Fill transmit FIFO */
    while (device->parent.reg->SR & SR_TNF && device->left)
    {
      device->parent.reg->DR = *device->txBuffer++;
      --device->left;
    }
    /* Disable transmit interrupt when all frames have been pushed to FIFO */
    if (!device->left)
      device->parent.reg->IMSC &= ~IMSC_TXIM;
  }
}
/*----------------------------------------------------------------------------*/
static enum result spiInit(void *object, const void *configPtr)
{
  const struct SpiConfig * const config = configPtr;
  struct Spi *device = object;
  struct SspConfig parentConfig = {
      .frame = 8 /* Fixed frame size */
  };
  enum result res;

  /* Check device configuration data */
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
  device->channelLock = MUTEX_UNLOCKED;
  device->deviceLock = MUTEX_UNLOCKED;

  /* Set pointer to hardware interrupt handler */
  device->parent.handler = interruptHandler;

  /* Set interrupt priority, lowest by default */
  NVIC_SetPriority(device->parent.irq, DEFAULT_PRIORITY);
  /* Enable UART interrupt */
  NVIC_EnableIRQ(device->parent.irq);
  return E_OK;
}
/*----------------------------------------------------------------------------*/
static void spiDeinit(void *object)
{
  struct Spi *device = object;

  NVIC_DisableIRQ(device->parent.irq);
  Ssp->deinit(device); /* Call SSP class destructor */
}
/*----------------------------------------------------------------------------*/
static uint32_t spiRead(void *object, uint8_t *buffer, uint32_t length)
{
  struct Spi *device = object;

  mutexLock(&device->channelLock);

  device->fill = length;
  /* Pop all previously received frames */
  while (device->parent.reg->SR & SR_RNE)
    (void)device->parent.reg->DR;

  /* Fill transmit FIFO with dummy frames */
  while (device->parent.reg->SR & SR_TNF && device->fill)
  {
    device->parent.reg->DR = 0xFF;
    --device->fill;
  }

  device->rxBuffer = buffer;
  device->left = length;
  /* Enable receive FIFO half full and receive FIFO timeout interrupts */
  device->parent.reg->IMSC |= IMSC_RXIM | IMSC_RTIM;
  /* Wait until all frames will be received */
  while (device->left || (device->parent.reg->SR & SR_BSY));

  mutexUnlock(&device->channelLock);

  return length;
}
/*----------------------------------------------------------------------------*/
static uint32_t spiWrite(void *object, const uint8_t *buffer, uint32_t length)
{
  struct Spi *device = object;

  mutexLock(&device->channelLock);

  device->left = length;
  /* Fill transmit FIFO */
  while (device->parent.reg->SR & SR_TNF && device->left)
  {
    device->parent.reg->DR = *buffer++;
    device->left--;
  }

  /* Enable transmit interrupt when packet size is greater than FIFO length */
  if (device->left)
  {
    device->txBuffer = buffer;
    device->parent.reg->IMSC |= IMSC_TXIM;
  }
  /* Wait until all frames will be transmitted */
  while (device->left || device->parent.reg->SR & SR_BSY);

  mutexUnlock(&device->channelLock);

  return length;
}
/*----------------------------------------------------------------------------*/
static enum result spiGet(void *object, enum ifOption option, void *data)
{
  switch (option)
  {
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
  struct Spi *device = object;

  switch (option)
  {
    case IF_DEVICE:
      if (*(uint32_t *)data)
        mutexLock(&device->deviceLock);
      else
        mutexUnlock(&device->deviceLock);
      return E_OK;
    case IF_RATE:
      sspSetRate(object, *(uint32_t *)data);
      return E_OK;
    case IF_PRIORITY:
      NVIC_SetPriority(device->parent.irq, *(uint32_t *)data);
      return E_OK;
    default:
      return E_ERROR;
  }
}
