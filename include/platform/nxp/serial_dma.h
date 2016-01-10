/*
 * platform/nxp/serial_dma.h
 * Copyright (C) 2012 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef PLATFORM_NXP_SERIAL_DMA_H_
#define PLATFORM_NXP_SERIAL_DMA_H_
/*----------------------------------------------------------------------------*/
#include <dma.h>
#include <libhalm/target.h>
/*----------------------------------------------------------------------------*/
#undef HEADER_PATH
#define HEADER_PATH <platform/PLATFORM_TYPE/GEN_UART/uart_base.h>
#include HEADER_PATH
#undef HEADER_PATH
/*----------------------------------------------------------------------------*/
extern const struct InterfaceClass * const SerialDma;
/*----------------------------------------------------------------------------*/
struct SerialDmaConfig
{
  /** Mandatory: baud rate. */
  uint32_t rate;
  /** Mandatory: serial input. */
  pinNumber rx;
  /** Mandatory: serial output. */
  pinNumber tx;
  /** Mandatory: peripheral identifier. */
  uint8_t channel;
  /** Mandatory: incoming data channel. */
  uint8_t rxChannel;
  /** Mandatory: outgoing data channel. */
  uint8_t txChannel;
  /** Optional: parity generation and checking. */
  enum uartParity parity;
};
/*----------------------------------------------------------------------------*/
struct SerialDma
{
  struct UartBase base;

  void (*callback)(void *);
  void *callbackArgument;

  /* DMA channel descriptors */
  struct Dma *rxDma, *txDma;
  /* Desired baud rate */
  uint32_t rate;
  /* Selection between blocking mode and zero copy mode */
  bool blocking;
};
/*----------------------------------------------------------------------------*/
#endif /* PLATFORM_NXP_SERIAL_DMA_H_ */
