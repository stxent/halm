/*
 * halm/platform/bouffalo/spi_dma.h
 * Copyright (C) 2026 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_BOUFFALO_SPI_DMA_H_
#define HALM_PLATFORM_BOUFFALO_SPI_DMA_H_
/*----------------------------------------------------------------------------*/
#include <halm/generic/spi.h>
#include <halm/platform/bouffalo/dma_base.h>
#include <halm/platform/bouffalo/spi_base.h>
/*----------------------------------------------------------------------------*/
extern const struct InterfaceClass * const SpiDma;

struct SpiDmaConfig
{
  /** Mandatory: serial data rate. */
  uint32_t rate;
  /** Optional: serial data input. */
  PinNumber miso;
  /** Optional: serial data output. */
  PinNumber mosi;
  /** Mandatory: serial clock output. */
  PinNumber sck;
  /** Mandatory: peripheral identifier. */
  uint8_t channel;
  /** Mandatory: mode number. */
  uint8_t mode;
  /** Mandatory: direct memory access channels. */
  uint8_t dma[2];
};

struct SpiDma
{
  struct SpiBase base;

  void (*callback)(void *);
  void *callbackArgument;

  /* Desired baud rate */
  uint32_t rate;

  /* DMA descriptor for RX channel */
  struct Dma *rxDma;
  /* DMA descriptor for TX channel */
  struct Dma *txDma;
  /* Pointer to an input buffer for bidirectional transfers */
  uint8_t *sink;

  /*
   * Dummy frame to be sent over transmit line in the receive mode or
   * to be received in the transmit mode.
   */
  uint8_t dummy;

  /* Selection between blocking mode and zero copy mode */
  bool blocking;
  /* Synchronize DMA TX and RX handlers */
  bool invoked;
  /* Selection between unidirectional and bidirectional modes */
  bool unidir;
};
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_BOUFFALO_SPI_DMA_H_ */
