/*
 * platform/nxp/spi.h
 * Copyright (C) 2013 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef SPI_H_
#define SPI_H_
/*----------------------------------------------------------------------------*/
#include <stdbool.h>
#include "ssp_base.h"
/*----------------------------------------------------------------------------*/
extern const struct InterfaceClass * const Spi;
/*----------------------------------------------------------------------------*/
struct SpiConfig
{
  /** Mandatory: serial data rate. */
  uint32_t rate;
  /** Optional: serial data input. */
  pin_t miso;
  /** Optional: serial data output. */
  pin_t mosi;
  /** Mandatory: serial clock output. */
  pin_t sck;
  /** Optional: interrupt priority. */
  priority_t priority;
  /** Mandatory: peripheral identifier. */
  uint8_t channel;
  /** Optional: mode number used in Serial Peripheral Interface mode. */
  uint8_t mode;
};
/*----------------------------------------------------------------------------*/
struct Spi
{
  struct SspBase parent;

  void (*callback)(void *);
  void *callbackArgument;

  /* Pointer to an input buffer */
  uint8_t *rxBuffer;
  /* Pointer to an output buffer */
  const uint8_t *txBuffer;
  /* Number of bytes to be received */
  uint32_t rxLeft;
  /* Number of bytes to be transmitted */
  uint32_t txLeft;

  /* Selection between blocking mode and zero copy mode */
  bool blocking;
};
/*----------------------------------------------------------------------------*/
#endif /* SPI_H_ */
