/*
 * halm/platform/lpc/gen_2/spi_dma.h
 * Copyright (C) 2025 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_LPC_SPI_DMA_H_
#error This header should not be included directly
#endif

#ifndef HALM_PLATFORM_LPC_GEN_2_SPI_DMA_H_
#define HALM_PLATFORM_LPC_GEN_2_SPI_DMA_H_
/*----------------------------------------------------------------------------*/
#include <halm/dma.h>
#include <halm/generic/spi.h>
#include <halm/platform/lpc/spi_base.h>
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
  /** Optional: DMA priority. Low priority is used by default. */
  uint8_t priority;
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

  /* Bytes to be received in interrupt mode */
  uint8_t awaiting;
  /* Dummy frame to be sent over the transmit line */
  uint8_t dummy;

  /* Enable blocking mode */
  bool blocking;
  /* Synchronize DMA TX and RX handlers */
  bool invoked;
  /* Enable unidirectional mode */
  bool unidir;
};
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_LPC_GEN_2_SPI_DMA_H_ */
