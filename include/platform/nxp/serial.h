/*
 * platform/nxp/serial.h
 * Copyright (C) 2012 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef SERIAL_H_
#define SERIAL_H_
/*----------------------------------------------------------------------------*/
#include <queue.h>
#include "uart_base.h"
/*----------------------------------------------------------------------------*/
extern const struct InterfaceClass *Serial;
/*----------------------------------------------------------------------------*/
struct SerialConfig
{
  uint32_t rxLength, txLength; /* Optional: queue lengths */
  uint32_t rate; /* Mandatory: baud rate */
  gpio_t rx, tx; /* Mandatory: RX and TX pins */
  uint8_t channel; /* Mandatory: Peripheral number */
  enum uartParity parity; /* Optional: even, odd or no parity */
};
/*----------------------------------------------------------------------------*/
struct Serial
{
  struct UartBase parent;

  /* Pointer to the callback function and to the callback argument */
  void (*callback)(void *);
  void *callbackArgument;

  /* Receive and transmit buffers */
  struct Queue rxQueue, txQueue;
};
/*----------------------------------------------------------------------------*/
#endif /* SERIAL_H_ */
