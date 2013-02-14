/*
 * serial.h
 * Copyright (C) 2012 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef SERIAL_H_
#define SERIAL_H_
/*----------------------------------------------------------------------------*/
#include "mutex.h"
#include "queue.h"
#include "uart.h"
/*----------------------------------------------------------------------------*/
extern const struct UartClass *Serial;
/*----------------------------------------------------------------------------*/
struct SerialConfig
{
  /* Mandatory arguments */
  uint8_t channel; /* Peripheral number */
  gpioKey rx, tx; /* RX and TX pins */
  uint32_t rate; /* Baud rate */
  /* Optional arguments */
  enum uartParity parity; /* Even, odd or no parity */
  uint32_t rxLength, txLength; /* Queue lengths */
};
/*----------------------------------------------------------------------------*/
struct Serial
{
  struct Uart parent;

  struct Queue rxQueue, txQueue; /* Receive and transmit buffers */
  Mutex queueLock; /* Access to queues */
};
/*----------------------------------------------------------------------------*/
#endif /* SERIAL_H_ */
