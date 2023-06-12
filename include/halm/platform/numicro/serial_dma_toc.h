/*
 * halm/platform/numicro/serial_dma_toc.h
 * Copyright (C) 2023 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_NUMICRO_SERIAL_DMA_TOC_H_
#define HALM_PLATFORM_NUMICRO_SERIAL_DMA_TOC_H_
/*----------------------------------------------------------------------------*/
#include <halm/generic/serial.h>
#include <halm/irq.h>
#include <halm/pin.h>
#include <halm/platform/numicro/pdma_base.h>
#include <xcore/interface.h>
#include <stdint.h>
/*----------------------------------------------------------------------------*/
extern const struct InterfaceClass * const SerialDmaTOC;

struct SerialDmaTOCConfig
{
  /**
   * Optional: memory buffer for transmit and receive queues. Queues will be
   * allocated on the heap when the pointer is set to zero.
   * Pointer address should be aligned.
   */
  void *arena;
  /** Mandatory: input buffer length. */
  size_t rxChunk;
  /** Mandatory: input queue size. */
  size_t rxLength;
  /** Mandatory: output queue size. */
  size_t txLength;
  /** Mandatory: baud rate. */
  uint32_t rate;
  /** Mandatory: timeout value in bit times. */
  uint32_t timeout;
  /** Optional: serial input. */
  PinNumber rx;
  /** Optional: serial output. */
  PinNumber tx;
  /** Optional: interrupt priority. */
  IrqPriority priority;
  /** Optional: parity bit setting. */
  enum SerialParity parity;
  /** Mandatory: peripheral identifier. */
  uint8_t channel;
  /**
   * Mandatory: direct memory access channels, one of the channels should
   * support timeout counter.
   */
  uint8_t dma[2];
};
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_NUMICRO_SERIAL_DMA_TOC_H_ */
