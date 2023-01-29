/*
 * halm/platform/stm32/serial.h
 * Copyright (C) 2016 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_STM32_SERIAL_H_
#define HALM_PLATFORM_STM32_SERIAL_H_
/*----------------------------------------------------------------------------*/
#include <halm/platform/stm32/uart_base.h>
/*----------------------------------------------------------------------------*/
extern const struct InterfaceClass * const Serial;

struct SerialConfig
{
  /** Mandatory: input queue size. */
  size_t rxLength;
  /** Mandatory: output queue size. */
  size_t txLength;
  /** Mandatory: baud rate. */
  uint32_t rate;
  /** Optional: parity bit setting. */
  enum SerialParity parity;
  /** Optional: serial input. */
  PinNumber rx;
  /** Optional: serial output. */
  PinNumber tx;
  /** Optional: interrupt priority. */
  IrqPriority priority;
  /** Mandatory: peripheral identifier. */
  uint8_t channel;
};
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_STM32_SERIAL_H_ */
