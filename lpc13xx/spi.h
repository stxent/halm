/*
 * spi.h
 * Copyright (C) 2013 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef SPI_IRQ_H_
#define SPI_IRQ_H_
/*----------------------------------------------------------------------------*/
#include "mutex.h"
#include "queue.h"
#include "ssp.h"
/*----------------------------------------------------------------------------*/
extern const struct SspClass *Spi;
/*----------------------------------------------------------------------------*/
struct SpiConfig
{
  /* Mandatory arguments */
  uint8_t channel; /* Peripheral number */
  gpioKey sck, miso, mosi; /* Serial clock, master output */
  uint32_t rate;
  /* Optional arguments */
  uint32_t rxLength, txLength; /* Queue lengths */
  uint8_t frame; /* Frame size, 8 bits by default */
  gpioKey cs; /* Chip select for slave operations */
};
/*----------------------------------------------------------------------------*/
struct Spi
{
  struct Ssp parent;

  struct Queue rxQueue, txQueue; /* Receive and transmit buffers */
  Mutex queueLock; /* Access to queues */
};
/*----------------------------------------------------------------------------*/
#endif /* SPI_IRQ_H_ */
