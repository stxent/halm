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
  uint32_t rate; /* Mandatory: serial data rate */
  uint32_t rxLength, txLength; /* Optional: queue lengths */
  gpioKey cs; /* Optional: chip select for slave mode */
  gpioKey sck, miso, mosi; /* Mandatory: peripheral pins */
  uint8_t channel; /* Mandatory: peripheral number */
};
/*----------------------------------------------------------------------------*/
struct Spi
{
  struct Ssp parent;

  struct Queue rxQueue, txQueue; /* Receive and transmit buffers */
  Mutex channelLock; /* Access to queues */
};
/*----------------------------------------------------------------------------*/
#endif /* SPI_IRQ_H_ */
