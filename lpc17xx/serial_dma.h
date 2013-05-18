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
extern const struct InterfaceClass *SerialDma;
/*----------------------------------------------------------------------------*/
struct SerialDmaConfig
{
  uint32_t rate; /* Mandatory: baud rate */
  gpioKey rx, tx; /* Mandatory: RX and TX pins */
  uint8_t channel; /* Mandatory: Peripheral number */
  /* TODO Make optional */
  int8_t rxChannel, txChannel; /* Mandatory: DMA channels */
  enum uartParity parity; /* Optional: even, odd or no parity */
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
