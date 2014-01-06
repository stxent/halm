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
  uint32_t rxLength, txLength; /* Optional: queue lengths */
  uint32_t rate; /* Mandatory: baud rate */
  gpio_t rx, tx; /* Mandatory: RX and TX pins */
  priority_t priority; /* Optional: interrupt priority */
  uint8_t channel; /* Mandatory: Peripheral identifier */
  enum uartParity parity; /* Optional: even, odd or no parity */
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
