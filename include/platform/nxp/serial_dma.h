/*
 * platform/nxp/serial_dma.h
 * Copyright (C) 2012 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef PLATFORM_NXP_SERIAL_DMA_H_
#define PLATFORM_NXP_SERIAL_DMA_H_
/*----------------------------------------------------------------------------*/
#include <containers/byte_queue.h>
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
  /** Mandatory: input queue size. */
  size_t rxLength;
  /** Mandatory: output queue size. */
  size_t txLength;
  /** Optional: parity generation and checking. */
  enum uartParity parity;
  /** Mandatory: serial input. */
  pinNumber rx;
  /** Mandatory: serial output. */
  pinNumber tx;
  /** Mandatory: peripheral identifier. */
  uint8_t channel;
  /** Mandatory: direct memory access channels. */
  uint8_t dma[2];
};
/*----------------------------------------------------------------------------*/
struct SerialDma
{
  struct UartBase base;

  void (*callback)(void *);
  void *callbackArgument;

  /* Desired baud rate */
  uint32_t rate;

  /* DMA channel descriptors */
  struct Dma *rxDma, *txDma;

  /* Bridges between interface functions and DMA operations */
  struct ByteQueue rxQueue, txQueue;

  /* Pointer to an allocated memory for temporary buffers */
  uint8_t *pool;
  /* Temporary buffers for DMA operations */
  uint8_t *rxBuffer, *txBuffer;
  /* Index of the first DMA buffer in the receive queue */
  uint8_t rxBufferIndex;
};
/*----------------------------------------------------------------------------*/
#endif /* PLATFORM_NXP_SERIAL_DMA_H_ */
