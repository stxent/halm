/*
 * halm/platform/stm32/spi.h
 * Copyright (C) 2018 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_STM32_SPI_H_
#define HALM_PLATFORM_STM32_SPI_H_
/*----------------------------------------------------------------------------*/
#include <halm/generic/spi.h>
#include <halm/platform/stm32/dma_oneshot.h>
#include <halm/platform/stm32/spi_base.h>
/*----------------------------------------------------------------------------*/
extern const struct InterfaceClass * const Spi;

struct SpiConfig
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
  /** Mandatory: number of the RX DMA stream. */
  uint8_t rxDma;
  /** Mandatory: number of the RX DMA stream. */
  uint8_t txDma;
};

struct Spi
{
  struct SpiBase base;

  void (*callback)(void *);
  void *callbackArgument;

  /* Desired baud rate */
  uint32_t rate;

  /* DMA channel for data reception */
  struct Dma *rxDma;
  /* DMA channel for data transmission */
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
#endif /* HALM_PLATFORM_STM32_SPI_H_ */
