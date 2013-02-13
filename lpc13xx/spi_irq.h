/*
 * spi_irq.h
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
extern const struct SspClass *SpiIrq;
/*----------------------------------------------------------------------------*/
struct SpiIrqConfig
{
  struct SspConfig ssp;
  uint32_t rxLength, txLength; /* Queue lengths */
};
/*----------------------------------------------------------------------------*/
struct SpiIrq
{
  struct Ssp parent;

  struct Queue rxQueue, txQueue; /* Receive and transmit buffers */
  Mutex queueLock; /* Access to queues */
};
/*----------------------------------------------------------------------------*/
#endif /* SPI_IRQ_H_ */
