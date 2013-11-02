/*
 * platform/nxp/spi_dma.h
 * Copyright (C) 2013 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef SPI_DMA_H_
#define SPI_DMA_H_
/*----------------------------------------------------------------------------*/
#include <dma.h>
#include <spinlock.h>
#include <platform/nxp/ssp_base.h>
/*----------------------------------------------------------------------------*/
extern const struct InterfaceClass *SpiDma;
/*----------------------------------------------------------------------------*/
struct SpiDmaConfig
{
  uint32_t rate; /* Mandatory: serial data rate */
  gpio_t miso, mosi, sck; /* Mandatory: peripheral pins */
  uint8_t channel; /* Mandatory: peripheral number */
  uint8_t mode; /* Optional: mode number used in SPI */
  int8_t rxChannel, txChannel; /* Mandatory: DMA channels */
};
/*----------------------------------------------------------------------------*/
struct SpiDma
{
  struct SspBase parent;

  /* Pointer to the callback function and to the callback argument */
  void (*callback)(void *);
  void *callbackArgument;

  /* DMA channel descriptors */
  struct Dma *rxDma, *txDma, *txMockDma;
  /* Exclusive access to the channel */
  spinlock_t lock;
  /* Selection between blocking mode and zero copy mode */
  bool blocking;
};
/*----------------------------------------------------------------------------*/
#endif /* SPI_DMA_H_ */
