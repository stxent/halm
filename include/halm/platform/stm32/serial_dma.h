/*
 * halm/platform/stm32/serial_dma.h
 * Copyright (C) 2021 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_STM32_SERIAL_DMA_H_
#define HALM_PLATFORM_STM32_SERIAL_DMA_H_
/*----------------------------------------------------------------------------*/
#include <halm/platform/stm32/dma_circular.h>
#include <halm/platform/stm32/dma_oneshot.h>
#include <halm/platform/stm32/uart_base.h>
#include <xcore/containers/byte_queue.h>
/*----------------------------------------------------------------------------*/
extern const struct InterfaceClass * const SerialDma;

struct SerialDmaConfig
{
  /** Mandatory: baud rate. */
  uint32_t rate;
  /** Mandatory: size of the circular buffer. */
  size_t rxChunk;
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
  /** Optional: interrupt priority. */
  IrqPriority priority;
  /** Mandatory: peripheral identifier. */
  uint8_t channel;
  /** Mandatory: number of the RX DMA stream. */
  uint8_t rxDma;
  /** Mandatory: number of the RX DMA stream. */
  uint8_t txDma;
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
  /* Pointer to the temporary reception buffer */
  uint8_t *rxBuffer;
  /* Position inside the temporary buffer */
  size_t rxPosition;

  /* Size of the circular reception buffer */
  size_t rxBufferSize;
  /* Size of the DMA TX transfer */
  size_t txBufferSize;
};
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_STM32_SERIAL_DMA_H_ */
