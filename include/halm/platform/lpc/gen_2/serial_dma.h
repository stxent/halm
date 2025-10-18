/*
 * halm/platform/lpc/gen_2/serial_dma.h
 * Copyright (C) 2025 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_LPC_SERIAL_DMA_H_
#error This header should not be included directly
#endif

#ifndef HALM_PLATFORM_LPC_GEN_2_SERIAL_DMA_H_
#define HALM_PLATFORM_LPC_GEN_2_SERIAL_DMA_H_
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
  /** Mandatory: number of input chunk. */
  size_t rxChunks;
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
  /**
   * Optional: oversampling value. Possible values range from 5 to 16.
   * In case the parameter is not initialized, the default value of 16 is used.
   */
  uint8_t oversampling;
  /** Optional: DMA priority. Low priority is used by default. */
  uint8_t priority;
};
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_LPC_GEN_2_SERIAL_DMA_H_ */
