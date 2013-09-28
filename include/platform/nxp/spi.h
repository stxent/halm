/*
 * spi.h
 * Copyright (C) 2013 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef SPI_H_
#define SPI_H_
/*----------------------------------------------------------------------------*/
#include "platform/nxp/spinlock.h"
#include "./ssp.h"
/*----------------------------------------------------------------------------*/
extern const struct InterfaceClass *Spi;
/*----------------------------------------------------------------------------*/
struct SpiConfig
{
  uint32_t rate; /* Mandatory: serial data rate */
  gpioKey cs; /* Optional: chip select for slave mode */
  gpioKey miso, mosi, sck; /* Mandatory: peripheral pins */
  uint8_t channel; /* Mandatory: peripheral number */
  uint8_t mode; /* Optional: mode number used in SPI */
};
/*----------------------------------------------------------------------------*/
struct Spi
{
  struct Ssp parent;

  void (*callback)(void *);
  void *callbackArgument;

  uint8_t *rxBuffer;
  const uint8_t *txBuffer;
  uint32_t left, fill;

  bool blocking; /* By default interface is in blocking mode */
  spinlock_t lock;
};
/*----------------------------------------------------------------------------*/
#endif /* SPI_H_ */
