/*
 * platform/nxp/gen_1/spi_dma.h
 * Copyright (C) 2013 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef PLATFORM_NXP_GEN_1_SPI_DMA_H_
#define PLATFORM_NXP_GEN_1_SPI_DMA_H_
/*----------------------------------------------------------------------------*/
#include <dma.h>
#include <platform/nxp/ssp_base.h>
/*----------------------------------------------------------------------------*/
extern const struct InterfaceClass * const SpiDma;
/*----------------------------------------------------------------------------*/
struct SpiDmaConfig
{
  /** Mandatory: serial data rate. */
  uint32_t rate;
  /** Optional: serial data input. */
  pinNumber miso;
  /** Optional: serial data output. */
  pinNumber mosi;
  /** Mandatory: serial clock output. */
  pinNumber sck;
  /** Mandatory: peripheral identifier. */
  uint8_t channel;
  /** Optional: mode number. */
  uint8_t mode;
  /**
   * Mandatory: direct memory access channels for incoming and outgoing data,
   * correct channel priority will be selected automatically.
   */
  uint8_t dma[2];
};
/*----------------------------------------------------------------------------*/
struct SpiDma
{
  struct SspBase base;

  void (*callback)(void *);
  void *callbackArgument;

  /* Direct memory access channel descriptors */
  struct Dma *rxDma, *txDma;
  /*
   * Dummy frame to be sent over transmit line in the receive mode or
   * to be received in the transmit mode. The peripheral descriptor structure
   * should be placed in auxiliary memory when direct memory access mode
   * is used in sleep mode because main memory may be unavailable for use.
   */
  uint8_t dummy;
  /* Selection between blocking mode and zero copy mode */
  bool blocking;
};
/*----------------------------------------------------------------------------*/
#endif /* PLATFORM_NXP_GEN_1_SPI_DMA_H_ */
