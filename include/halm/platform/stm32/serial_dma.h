/*
 * halm/platform/stm32/serial_dma.h
 * Copyright (C) 2021 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_STM32_SERIAL_DMA_H_
#define HALM_PLATFORM_STM32_SERIAL_DMA_H_
/*----------------------------------------------------------------------------*/
#include <halm/platform/stm32/uart_base.h>
/*----------------------------------------------------------------------------*/
extern const struct InterfaceClass * const SerialDma;

struct SerialDmaConfig
{
  /** Mandatory: size of the circular buffer. */
  size_t rxChunk;
  /** Mandatory: input queue size. */
  size_t rxLength;
  /** Mandatory: output queue size. */
  size_t txLength;
  /** Mandatory: baud rate. */
  uint32_t rate;
  /** Mandatory: serial input. */
  PinNumber rx;
  /** Mandatory: serial output. */
  PinNumber tx;
  /** Optional: parity bit setting. */
  enum SerialParity parity;
  /** Mandatory: peripheral identifier. */
  uint8_t channel;
  /** Mandatory: number of the RX DMA stream. */
  uint8_t rxDma;
  /** Mandatory: number of the RX DMA stream. */
  uint8_t txDma;
};
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_STM32_SERIAL_DMA_H_ */
