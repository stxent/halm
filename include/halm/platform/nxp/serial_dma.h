/*
 * halm/platform/nxp/serial_dma.h
 * Copyright (C) 2012 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef HALM_PLATFORM_NXP_SERIAL_DMA_H_
#define HALM_PLATFORM_NXP_SERIAL_DMA_H_
/*----------------------------------------------------------------------------*/
#include <xcore/containers/byte_queue.h>
#include <halm/dma.h>
#include <halm/target.h>
/*----------------------------------------------------------------------------*/
#undef HEADER_PATH
#define HEADER_PATH <halm/platform/PLATFORM_TYPE/GEN_UART/uart_base.h>
#include HEADER_PATH
#undef HEADER_PATH
/*----------------------------------------------------------------------------*/
extern const struct InterfaceClass * const SerialDma;

struct SerialDmaConfig
{
  /** Mandatory: baud rate. */
  uint32_t rate;
  /** Mandatory: size of the input buffer, input is double buffered. */
  size_t rxChunks;
  /** Mandatory: input queue size. */
  size_t rxLength;
  /** Mandatory: output queue size. */
  size_t txLength;
  /** Optional: parity bit setting. */
  enum SerialParity parity;
  /** Mandatory: serial input. */
  PinNumber rx;
  /** Mandatory: serial output. */
  PinNumber tx;
  /** Mandatory: peripheral identifier. */
  uint8_t channel;
  /** Mandatory: direct memory access channels. */
  uint8_t dma[2];
};

struct SerialDma
{
  struct UartBase base;

  void (*callback)(void *);
  void *callbackArgument;

  /* Desired baud rate */
  uint32_t rate;

  /* DMA channel descriptors */
  struct Dma *rxDma;
  struct Dma *txDma;

  /* Input and output queues */
  struct ByteQueue rxQueue;
  struct ByteQueue txQueue;

  /* DMA RX chunk count */
  size_t rxChunks;
  /* Size of the single DMA RX buffer */
  size_t rxBufferSize;
  /* Size of the DMA TX transfer */
  size_t txBufferSize;
};
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_NXP_SERIAL_DMA_H_ */
