/*
 * halm/platform/nxp/serial.h
 * Copyright (C) 2012 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef HALM_PLATFORM_NXP_SERIAL_H_
#define HALM_PLATFORM_NXP_SERIAL_H_
/*----------------------------------------------------------------------------*/
#include <xcore/containers/byte_queue.h>
#include <halm/target.h>
/*----------------------------------------------------------------------------*/
#undef HEADER_PATH
#define HEADER_PATH <halm/platform/PLATFORM_TYPE/GEN_UART/uart_base.h>
#include HEADER_PATH
#undef HEADER_PATH
/*----------------------------------------------------------------------------*/
extern const struct InterfaceClass * const Serial;
/*----------------------------------------------------------------------------*/
struct SerialConfig
{
  /** Mandatory: baud rate. */
  uint32_t rate;
  /** Mandatory: input queue size. */
  size_t rxLength;
  /** Mandatory: output queue size. */
  size_t txLength;
  /** Optional: parity generation and checking. */
  enum uartParity parity;
  /** Mandatory: serial input. */
  PinNumber rx;
  /** Mandatory: serial output. */
  PinNumber tx;
  /** Optional: interrupt priority. */
  IrqPriority priority;
  /** Mandatory: peripheral identifier. */
  uint8_t channel;
};
/*----------------------------------------------------------------------------*/
struct Serial
{
  struct UartBase base;

  void (*callback)(void *);
  void *callbackArgument;

  /* Input queue */
  struct ByteQueue rxQueue;
  /* Output queue */
  struct ByteQueue txQueue;
  /* Desired baud rate */
  uint32_t rate;
};
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_NXP_SERIAL_H_ */
