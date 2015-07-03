/*
 * platform/nxp/one_wire_uart.h
 * Copyright (C) 2013 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef PLATFORM_NXP_ONE_WIRE_UART_H_
#define PLATFORM_NXP_ONE_WIRE_UART_H_
/*----------------------------------------------------------------------------*/
#include <containers/byte_queue.h>
#include <libhalm/target.h>
/*----------------------------------------------------------------------------*/
#undef HEADER_PATH
#define HEADER_PATH <platform/PLATFORM_TYPE/GEN_UART/uart_base.h>
#include HEADER_PATH
#undef HEADER_PATH
/*----------------------------------------------------------------------------*/
extern const struct InterfaceClass * const OneWireUart;
/*----------------------------------------------------------------------------*/
struct OneWireUartConfig
{
  /** Mandatory: serial input. */
  pin_t rx;
  /** Mandatory: serial output. */
  pin_t tx;
  /** Optional: interrupt priority. */
  priority_t priority;
  /** Mandatory: peripheral identifier. */
  uint8_t channel;
};
/*----------------------------------------------------------------------------*/
struct OneWireUart
{
  struct UartBase parent;

  void (*callback)(void *);
  void *callbackArgument;

  /* Pointer to an input buffer */
  uint8_t *rxBuffer;
  /* Address of the device */
  uint64_t address;
  /* Number of bytes to be transmitted */
  uint8_t left;
  /* Position in a receiving word and temporary buffer for this word */
  uint8_t bit, word;
  /* Output queue containing command, address and data */
  struct ByteQueue txQueue;

  /* Computed data rates for reset and data transmission modes */
  struct UartRateConfig dataRate, resetRate;

  /* Current interface state */
  uint8_t state;
  /* Selection between blocking mode and zero copy mode */
  bool blocking;
};
/*----------------------------------------------------------------------------*/
#endif /* PLATFORM_NXP_ONE_WIRE_UART_H_ */
