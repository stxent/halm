/*
 * halm/platform/lpc/serial_dma.h
 * Copyright (C) 2012 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_LPC_SERIAL_DMA_H_
#define HALM_PLATFORM_LPC_SERIAL_DMA_H_
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
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_LPC_SERIAL_DMA_H_ */
