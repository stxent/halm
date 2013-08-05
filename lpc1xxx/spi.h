/*
 * spi.h
 * Copyright (C) 2013 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef SPI_H_
#define SPI_H_
/*----------------------------------------------------------------------------*/
#include "mutex.h"
#include "ssp.h"
/*----------------------------------------------------------------------------*/
extern const struct InterfaceClass *Spi;
/*----------------------------------------------------------------------------*/
struct SpiConfig
{
  uint32_t rate; /* Mandatory: serial data rate */
  gpioKey cs; /* Optional: chip select for slave mode */
  gpioKey sck, miso, mosi; /* Mandatory: peripheral pins */
  uint8_t channel; /* Mandatory: peripheral number */
};
/*----------------------------------------------------------------------------*/
struct Spi
{
  struct Ssp parent;

  uint8_t *rxBuffer;
  const uint8_t *txBuffer;
  uint32_t left, fill;

  void (*callback)(void *); /* Function called on completion event */
  void *callbackArgument;

  bool blocking; /* By default interface is in blocking mode */
  Mutex channelLock; /* Access to channel */
};
/*----------------------------------------------------------------------------*/
#endif /* SPI_H_ */
