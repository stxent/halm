/*
 * halm/platform/lpc/one_wire_uart.h
 * Copyright (C) 2013 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef HALM_PLATFORM_LPC_ONE_WIRE_UART_H_
#define HALM_PLATFORM_LPC_ONE_WIRE_UART_H_
/*----------------------------------------------------------------------------*/
#include <halm/generic/one_wire.h>
#include <halm/target.h>
#include <xcore/containers/byte_queue.h>
/*----------------------------------------------------------------------------*/
#undef HEADER_PATH
#define HEADER_PATH <halm/platform/PLATFORM_TYPE/GEN_UART/uart_base.h>
#include HEADER_PATH
#undef HEADER_PATH
/*----------------------------------------------------------------------------*/
extern const struct InterfaceClass * const OneWireUart;

struct OneWireUartConfig
{
  /** Mandatory: serial input. */
  PinNumber rx;
  /** Mandatory: serial output. */
  PinNumber tx;
  /** Optional: interrupt priority. */
  IrqPriority priority;
  /** Mandatory: peripheral identifier. */
  uint8_t channel;
};

struct OneWireUart
{
  struct UartBase base;

  void (*callback)(void *);
  void *callbackArgument;

  /* Address of the device */
  uint64_t address;

  /* Pointer to an input buffer */
  uint8_t *rxBuffer;
  /* Output queue containing command, address and data */
  struct ByteQueue txQueue;

  /* Number of bytes to be transmitted */
  uint8_t left;
  /* Position in a receiving word */
  uint8_t bit;
  /* Temporary buffer for receiving word */
  uint8_t word;

  /* Current interface state */
  uint8_t state;
  /* Selection between blocking mode and zero copy mode */
  bool blocking;

  /* Computed data rates for reset and data transmission modes */
  struct UartRateConfig dataRate, resetRate;
};
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_LPC_ONE_WIRE_UART_H_ */
