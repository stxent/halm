/*
 * serial.h
 * Copyright (C) 2012 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef SERIAL_DMA_H_
#define SERIAL_DMA_H_
/*----------------------------------------------------------------------------*/
#include "dma.h"
#include "mutex.h"
#include "uart.h"
/*----------------------------------------------------------------------------*/
extern const struct UartClass *SerialDma;
/*----------------------------------------------------------------------------*/
struct SerialDmaConfig
{
  /* Mandatory arguments */
  uint8_t channel; /* Peripheral number */
  gpioKey rx, tx; /* RX and TX pins */
  uint32_t rate; /* Baud rate */
  /* Optional arguments */
  enum uartParity parity; /* Even, odd or no parity */
  int8_t rxChannel, txChannel; /* DMA channels */ /* TODO Make optional */
};
/*----------------------------------------------------------------------------*/
struct SerialDma
{
  struct Uart parent;

  struct Dma *rxDma, *txDma;
  Mutex dmaLock; /* Access to DMA channels */
};
/*----------------------------------------------------------------------------*/
#endif /* SERIAL_DMA_H_ */
