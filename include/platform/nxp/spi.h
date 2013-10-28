/*
 * platform/nxp/spi.h
 * Copyright (C) 2013 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef SPI_H_
#define SPI_H_
/*----------------------------------------------------------------------------*/
#include <spinlock.h>
#include "ssp_base.h"
/*----------------------------------------------------------------------------*/
extern const struct InterfaceClass *Spi;
/*----------------------------------------------------------------------------*/
struct SpiConfig
{
  uint32_t rate; /* Mandatory: serial data rate */
  gpio_t cs; /* Optional: chip select for slave mode */
  gpio_t miso, mosi, sck; /* Mandatory: peripheral pins */
  priority_t priority; /* Optional: interrupt priority */
  uint8_t channel; /* Mandatory: peripheral number */
  uint8_t mode; /* Optional: mode number used in SPI */
};
/*----------------------------------------------------------------------------*/
struct Spi
{
  struct SspBase parent;

  void (*callback)(void *);
  void *callbackArgument;

  /* Pointer used in read mode */
  uint8_t *rxBuffer;
  /* Pointer used in write mode */
  const uint8_t *txBuffer;
  /* Number of bytes to be received */
  volatile uint32_t left;
  /* Number of bytes to be transmitted */
  uint32_t length;

  /* Exclusive access to the channel */
  spinlock_t lock;
  /* Selection between blocking mode and zero copy mode */
  bool blocking;
};
/*----------------------------------------------------------------------------*/
#endif /* SPI_H_ */
