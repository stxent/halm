/*
 * halm/platform/numicro/serial_dma.h
 * Copyright (C) 2023 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_NUMICRO_SERIAL_DMA_H_
#define HALM_PLATFORM_NUMICRO_SERIAL_DMA_H_
/*----------------------------------------------------------------------------*/
#include <halm/generic/serial.h>
#include <halm/irq.h>
#include <halm/pin.h>
#include <halm/platform/numicro/pdma_base.h>
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
  /** Mandatory: number of input chunks. */
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
  /** Optional: interrupt priority. */
  IrqPriority priority;
  /** Mandatory: peripheral identifier. */
  uint8_t channel;
  /**
   * Mandatory: DMA channels used for incoming and outgoing data, the channel
   * with higher priority will be used for reception.
   */
  uint8_t dma[2];
};
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_NUMICRO_SERIAL_DMA_H_ */
