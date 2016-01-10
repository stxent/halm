/*
 * platform/nxp/gen_1/spi.h
 * Copyright (C) 2013 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef PLATFORM_NXP_GEN_1_SPI_H_
#define PLATFORM_NXP_GEN_1_SPI_H_
/*----------------------------------------------------------------------------*/
#include <stdbool.h>
#include <platform/nxp/ssp_base.h>
/*----------------------------------------------------------------------------*/
extern const struct InterfaceClass * const Spi;
/*----------------------------------------------------------------------------*/
struct SpiConfig
{
  /** Mandatory: serial data rate. */
  uint32_t rate;
  /** Optional: serial data input. */
  pinNumber miso;
  /** Optional: serial data output. */
  pinNumber mosi;
  /** Mandatory: serial clock output. */
  pinNumber sck;
  /** Optional: interrupt priority. */
  irqPriority priority;
  /** Mandatory: peripheral identifier. */
  uint8_t channel;
  /** Optional: mode number. */
  uint8_t mode;
};
/*----------------------------------------------------------------------------*/
struct Spi
{
  struct SspBase base;

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
#endif /* PLATFORM_NXP_GEN_1_SPI_H_ */
