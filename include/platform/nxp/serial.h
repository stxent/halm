/*
 * platform/nxp/serial.h
 * Copyright (C) 2012 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef SERIAL_H_
#define SERIAL_H_
/*----------------------------------------------------------------------------*/
#include <byte_queue.h>
#include "uart_base.h"
/*----------------------------------------------------------------------------*/
extern const struct InterfaceClass *Serial;
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
  gpio_t rx;
  /** Mandatory: serial output. */
  gpio_t tx;
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
};
/*----------------------------------------------------------------------------*/
#endif /* SERIAL_H_ */
