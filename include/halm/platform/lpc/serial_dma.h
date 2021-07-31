/*
 * halm/platform/lpc/serial_dma.h
 * Copyright (C) 2012 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_LPC_SERIAL_DMA_H_
#define HALM_PLATFORM_LPC_SERIAL_DMA_H_
/*----------------------------------------------------------------------------*/
#include <halm/dma.h>
#include <halm/platform/lpc/uart_base.h>
#include <xcore/containers/byte_queue.h>
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
  /** Optional: memory arena for queues. */
  void *arena;
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

  /* DMA channel for data reception */
  struct Dma *rxDma;
  /* DMA channel for data transmission */
  struct Dma *txDma;

  /* Input queue */
  struct ByteQueue rxQueue;
  /* Output queue */
  struct ByteQueue txQueue;
  /* Queues are allocated in a static memory */
  bool preallocated;

  /* Count of reception buffers */
  size_t rxChunks;
  /* Position inside the current reception buffer */
  size_t rxPosition;
  /* Size of the single reception buffer */
  size_t rxBufferSize;
  /* Size of the current outgoing transfer */
  size_t txBufferSize;
};
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_LPC_SERIAL_DMA_H_ */
