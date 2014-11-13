/*
 * platform/nxp/serial.h
 * Copyright (C) 2012 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef PLATFORM_NXP_SERIAL_H_
#define PLATFORM_NXP_SERIAL_H_
/*----------------------------------------------------------------------------*/
#include <byte_queue.h>
#include <mcu.h>
/*----------------------------------------------------------------------------*/
#undef HEADER_PATH
#define HEADER_PATH <platform/PLATFORM_TYPE/GENERATION/uart_base.h>
#include HEADER_PATH
/*----------------------------------------------------------------------------*/
extern const struct InterfaceClass * const Serial;
/*----------------------------------------------------------------------------*/
struct SerialConfig
{
  /** Optional: input queue size. */
  uint32_t rxLength;
  /** Optional: output queue size. */
  uint32_t txLength;
  /** Mandatory: baud rate. */
  uint32_t rate;
  /** Mandatory: serial input. */
  pin_t rx;
  /** Mandatory: serial output. */
  pin_t tx;
  /** Optional: interrupt priority. */
  priority_t priority;
  /** Mandatory: peripheral identifier. */
  uint8_t channel;
  /** Optional: parity generation and checking. */
  enum uartParity parity;
};
/*----------------------------------------------------------------------------*/
struct Serial
{
  struct UartBase parent;

  void (*callback)(void *);
  void *callbackArgument;

  /* Input and output queues */
  struct ByteQueue rxQueue, txQueue;
  /* Desired baud rate */
  uint32_t rate;
};
/*----------------------------------------------------------------------------*/
#endif /* PLATFORM_NXP_SERIAL_H_ */
