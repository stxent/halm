/*
 * platform/nxp/serial_dma.h
 * Copyright (C) 2012 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef SERIAL_DMA_H_
#define SERIAL_DMA_H_
/*----------------------------------------------------------------------------*/
#include <dma.h>
#include <platform/nxp/uart_base.h>
/*----------------------------------------------------------------------------*/
extern const struct InterfaceClass *SerialDma;
/*----------------------------------------------------------------------------*/
struct SerialDmaConfig
{
  uint32_t rate; /* Mandatory: baud rate */
  gpio_t rx, tx; /* Mandatory: RX and TX pins */
  uint8_t channel; /* Mandatory: peripheral identifier */
  /* TODO Make optional */
  int8_t rxChannel, txChannel; /* Mandatory: DMA channels */
  enum uartParity parity; /* Optional: even, odd or no parity */
};
/*----------------------------------------------------------------------------*/
struct SerialDma
{
  struct UartBase parent;

  void (*callback)(void *);
  void *callbackArgument;

  /* DMA channel descriptors */
  struct Dma *rxDma, *txDma;
  /* Selection between blocking mode and zero copy mode */
  bool blocking;
};
/*----------------------------------------------------------------------------*/
#endif /* SERIAL_DMA_H_ */
