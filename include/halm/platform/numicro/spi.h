/*
 * halm/platform/numicro/spi.h
 * Copyright (C) 2023 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_NUMICRO_SPI_H_
#define HALM_PLATFORM_NUMICRO_SPI_H_
/*----------------------------------------------------------------------------*/
#include <halm/generic/spi.h>
#include <halm/platform/numicro/spi_base.h>
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
};

struct Spi
{
  struct SpiBase base;

  void (*callback)(void *);
  void *callbackArgument;

  /* Desired baud rate */
  uint32_t rate;
  /* Pointer to an input buffer */
  uint8_t *rxBuffer;
  /* Pointer to an output buffer */
  const uint8_t *txBuffer;
  /* Number of bytes to be received */
  size_t rxLeft;
  /* Number of bytes to be transmitted */
  size_t txLeft;

  /* Selection between blocking mode and zero copy mode */
  bool blocking;
  /* Selection between unidirectional and bidirectional modes */
  bool unidir;
};
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_NUMICRO_SPI_H_ */
