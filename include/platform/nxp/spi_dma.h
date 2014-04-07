/*
 * platform/nxp/spi_dma.h
 * Copyright (C) 2013 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef SPI_DMA_H_
#define SPI_DMA_H_
/*----------------------------------------------------------------------------*/
#include <dma.h>
#include <platform/nxp/ssp_base.h>
/*----------------------------------------------------------------------------*/
extern const struct InterfaceClass *SpiDma;
/*----------------------------------------------------------------------------*/
struct SpiDmaConfig
{
  /** Mandatory: serial data rate. */
  uint32_t rate;
  /** Mandatory: data input. */
  gpio_t miso;
  /** Mandatory: data output. */
  gpio_t mosi; /* Mandatory: peripheral pins */
  /** Mandatory: serial clock output. */
  gpio_t sck;
  /** Mandatory: peripheral identifier. */
  uint8_t channel;
  /** Optional: mode number of the Serial Peripheral Interface. */
  uint8_t mode;
  /** Mandatory: incoming data channel. */
  uint8_t rxChannel;
  /** Mandatory: outgoing data channel. */
  uint8_t txChannel;
};
/*----------------------------------------------------------------------------*/
struct SpiDma
{
  struct SspBase parent;

  void (*callback)(void *);
  void *callbackArgument;

  /* DMA channel descriptors */
  struct Dma *rxDma, *txDma, *txMockDma;
  /* Selection between blocking mode and zero copy mode */
  bool blocking;
};
/*----------------------------------------------------------------------------*/
#endif /* SPI_DMA_H_ */
