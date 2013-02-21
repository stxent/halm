/*
 * spi.c
 * Copyright (C) 2013 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include "spi.h"
#include "ssp_defs.h"
/*----------------------------------------------------------------------------*/
/* SPI settings */
#define DEFAULT_PRIORITY                15 /* Lowest priority in Cortex-M3 */
/*----------------------------------------------------------------------------*/
enum cleanup
{
  FREE_NONE = 0,
  FREE_PARENT,
  FREE_RX_QUEUE,
  FREE_TX_QUEUE,
  FREE_ALL
};
/*----------------------------------------------------------------------------*/
static void spiCleanup(struct Spi *, enum cleanup);
/*----------------------------------------------------------------------------*/
static void spiHandler(void *);
static enum result spiInit(void *, const void *);
static void spiDeinit(void *);
static uint32_t spiRead(void *, uint8_t *, uint32_t);
static uint32_t spiWrite(void *, const uint8_t *, uint32_t);
static enum result spiGetOpt(void *, enum ifOption, void *);
static enum result spiSetOpt(void *, enum ifOption, const void *);
/*----------------------------------------------------------------------------*/
/* We like structures so we put a structure in a structure */
/* So we can initialize a structure while we initialize a structure */
static const struct SspClass spiTable = {
    .parent = {
        .size = sizeof(struct Spi),
        .init = spiInit,
        .deinit = spiDeinit,

        .read = spiRead,
        .write = spiWrite,
        .getopt = spiGetOpt,
        .setopt = spiSetOpt
    },
    .handler = spiHandler
};
/*----------------------------------------------------------------------------*/
const struct SspClass *Spi = &spiTable;
/*----------------------------------------------------------------------------*/
static void spiCleanup(struct Spi *device, enum cleanup step)
{
  switch (step)
  {
    case FREE_ALL:
      NVIC_DisableIRQ(device->parent.irq); /* Disable interrupt */
    case FREE_TX_QUEUE:
      queueDeinit(&device->txQueue);
    case FREE_RX_QUEUE:
      queueDeinit(&device->rxQueue);
    case FREE_PARENT:
      Spi->parent.deinit(device); /* Call SSP class destructor */
      break;
    default:
      break;
  }
}
/*----------------------------------------------------------------------------*/
static void spiHandler(void *object)
{
  struct Spi *device = object;
  uint16_t data;
  uint8_t status = device->parent.reg->MIS;

  if (status & MIS_RXMIS)
  {
    /* Frame will be removed from FIFO after reading from DR register */
    while (device->parent.reg->SR & SR_RNE) //FIXME Queue overflow
    {
      data = device->parent.reg->DR;
      /* Received frames will be dropped when queue becomes full */
      if (!queueFull(&device->rxQueue))
        queuePush(&device->rxQueue, (uint8_t)data); //FIXME Change type
    }
  }
  if (status & MIS_TXMIS)
  {
    /* Fill transmit FIFO */
    while (queueSize(&device->txQueue) && device->parent.reg->SR & SR_TNF)
      device->parent.reg->DR = queuePop(&device->txQueue);
  }
}
/*----------------------------------------------------------------------------*/
static enum result spiInit(void *object, const void *configPtr)
{
  /* Set pointer to device configuration data */
  const struct SpiConfig *config = configPtr;
  struct Spi *device = object;
  const struct SspConfig parentConfig = {
      .channel = config->channel,
      .sck = config->sck,
      .miso = config->miso,
      .mosi = config->mosi,
      .rate = config->rate,
      .frame = config->frame,
      .cs = config->cs
  };
  enum result res;

  /* Call SSP class constructor */
  if ((res = Ssp->parent.init(object, &parentConfig)) != E_OK)
    return res;

  /* Initialize RX and TX queues */
  if ((res = queueInit(&device->rxQueue, config->rxLength)) != E_OK)
  {
    spiCleanup(device, FREE_PARENT);
    return res;
  }
  if ((res = queueInit(&device->txQueue, config->txLength)) != E_OK)
  {
    spiCleanup(device, FREE_RX_QUEUE);
    return res;
  }

  device->queueLock = MUTEX_UNLOCKED;

  /* Enable receive half-full and transmit half-empty interrupts */
//  device->parent.reg->IMSC = IMSC_RXIM | IMSC_TXIM;

  /* Set interrupt priority, lowest by default */
  NVIC_SetPriority(device->parent.irq, DEFAULT_PRIORITY);
  /* Enable UART interrupt */
  NVIC_EnableIRQ(device->parent.irq);
  return E_OK;
}
/*----------------------------------------------------------------------------*/
static void spiDeinit(void *object)
{
  spiCleanup(object, FREE_ALL);
}
/*----------------------------------------------------------------------------*/
static uint32_t spiRead(void *object, uint8_t *buffer, uint32_t length)
{
  struct Spi *device = object;
  uint32_t read = 0;

  mutexLock(&device->queueLock);
  NVIC_DisableIRQ(device->parent.irq);
  while (queueSize(&device->rxQueue) && read < length)
  {
    *buffer++ = queuePop(&device->rxQueue);
    read++;
  }
  NVIC_EnableIRQ(device->parent.irq);
  mutexUnlock(&device->queueLock);
  return read;
}
/*----------------------------------------------------------------------------*/
static uint32_t spiWrite(void *object, const uint8_t *buffer, uint32_t length)
{
  struct Spi *device = object;
  uint32_t written = 0;

  mutexLock(&device->queueLock);
  NVIC_DisableIRQ(device->parent.irq);
  /* Check transmitter state */
  if (!(device->parent.reg->SR & SR_BSY) && queueEmpty(&device->txQueue))
  {
    /* Transmitter is idle, fill TX FIFO */
    while (device->parent.reg->SR & SR_TNF && written < length)
    {
      device->parent.reg->DR = *buffer++;
      written++;
    }
  }
  /* Fill TX queue with the rest of data */
  while (!queueFull(&device->txQueue) && written < length)
  {
    queuePush(&device->txQueue, *buffer++);
    written++;
  }
  NVIC_EnableIRQ(device->parent.irq);
  mutexUnlock(&device->queueLock);
  return written;
}
/*----------------------------------------------------------------------------*/
static enum result spiGetOpt(void *object, enum ifOption option, void *data)
{
  return E_ERROR;
}
/*----------------------------------------------------------------------------*/
static enum result spiSetOpt(void *object, enum ifOption option,
    const void *data)
{
  struct Spi *device = object;

  switch (option)
  {
    case IF_SPEED:
      sspSetRate(object, *(uint32_t *)data);
      return E_OK;
    case IF_PRIORITY:
      NVIC_SetPriority(device->parent.irq, *(uint32_t *)data);
      return E_OK;
    default:
      return E_ERROR;
  }
}
