/*
 * halm/platform/lpc/gen_2/serial.h
 * Copyright (C) 2025 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_LPC_SERIAL_H_
#error This header should not be included directly
#endif

#ifndef HALM_PLATFORM_LPC_GEN_2_SERIAL_H_
#define HALM_PLATFORM_LPC_GEN_2_SERIAL_H_
/*----------------------------------------------------------------------------*/
#include <halm/generic/serial.h>
#include <halm/irq.h>
#include <halm/pin.h>
#include <xcore/interface.h>
#include <stdint.h>
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
   * Optional: oversampling value. Possible values range from 5 to 16.
   * In case the parameter is not initialized, the default value of 16 is used.
   */
  uint8_t oversampling;
};
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_LPC_GEN_2_SERIAL_H_ */
