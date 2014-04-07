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
  /** Mandatory: baud rate. */
  uint32_t rate;
  /** Mandatory: Receive Data pin. */
  gpio_t rx;
  /** Mandatory: Transmit Data pin. */
  gpio_t tx;
  /** Mandatory: peripheral identifier. */
  uint8_t channel;
  /** Mandatory: incoming data channel. */
  uint8_t rxChannel;
  /** Mandatory: outgoing data channel. */
  uint8_t txChannel;
  /** Optional: parity configuration option. */
  enum uartParity parity;
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
