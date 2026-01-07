/*
 * halm/platform/bouffalo/serial_dma.h
 * Copyright (C) 2026 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_BOUFFALO_SERIAL_DMA_H_
#define HALM_PLATFORM_BOUFFALO_SERIAL_DMA_H_
/*----------------------------------------------------------------------------*/
#include <halm/generic/serial.h>
#include <halm/irq.h>
#include <halm/pin.h>
#include <xcore/interface.h>
#include <stdint.h>
/*----------------------------------------------------------------------------*/
extern const struct InterfaceClass * const SerialDma;

struct SerialDmaConfig
{
  /**
   * Optional: memory buffer for transmit and receive queues. Queues will be
   * allocated on the heap when the pointer is set to zero.
   * Pointer address should be aligned.
   */
  void *arena;
  /** Mandatory: input buffer size. */
  size_t rxChunk;
  /** Mandatory: input queue size. */
  size_t rxLength;
  /** Mandatory: output queue size. */
  size_t txLength;
  /** Mandatory: baud rate. */
  uint32_t rate;
  /** Optional: serial input. */
  PinNumber rx;
  /** Optional: serial output. */
  PinNumber tx;
  /** Optional: parity bit setting. */
  enum SerialParity parity;
  /** Mandatory: peripheral identifier. */
  uint8_t channel;
  /** Mandatory: direct memory access channels. */
  uint8_t dma[2];
};
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_BOUFFALO_SERIAL_DMA_H_ */
