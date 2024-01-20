/*
 * halm/platform/lpc/gen_1/spi_dma.h
 * Copyright (C) 2013 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_LPC_SPI_DMA_H_
#error This header should not be included directly
#endif

#ifndef HALM_PLATFORM_LPC_GEN_1_SPI_DMA_H_
#define HALM_PLATFORM_LPC_GEN_1_SPI_DMA_H_
/*----------------------------------------------------------------------------*/
#include <halm/dma.h>
#include <halm/generic/spi.h>
#include <halm/platform/lpc/ssp_base.h>
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
  /** Optional: interrupt priority. */
  IrqPriority priority;
  /** Mandatory: peripheral identifier. */
  uint8_t channel;
  /** Mandatory: mode number. */
  uint8_t mode;
  /**
   * Mandatory: direct memory access channels for incoming and outgoing data,
   * correct channel priority will be selected automatically.
   */
  uint8_t dma[2];
};

struct SpiDma
{
  struct SspBase base;

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

  /* Bytes to be received in interrupt mode */
  uint8_t awaiting;
  /*
   * Dummy frame to be sent over transmit line in the receive mode or
   * to be received in the transmit mode. The peripheral descriptor structure
   * should be placed in auxiliary memory when direct memory access mode
   * is used in sleep mode because main memory may be unavailable for use.
   */
  uint8_t dummy;

  /* Enable blocking mode */
  bool blocking;
  /* Synchronize DMA TX and RX handlers */
  bool invoked;
  /* Enable unidirectional mode */
  bool unidir;
};
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_LPC_GEN_1_SPI_DMA_H_ */
