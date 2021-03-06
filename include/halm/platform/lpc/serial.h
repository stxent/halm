/*
 * halm/platform/lpc/serial.h
 * Copyright (C) 2012 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_LPC_SERIAL_H_
#define HALM_PLATFORM_LPC_SERIAL_H_
/*----------------------------------------------------------------------------*/
#include <halm/platform/lpc/uart_base.h>
#include <xcore/containers/byte_queue.h>
/*----------------------------------------------------------------------------*/
extern const struct InterfaceClass * const Serial;

struct SerialConfig
{
  /** Mandatory: baud rate. */
  uint32_t rate;
  /** Mandatory: input queue size. */
  size_t rxLength;
  /** Mandatory: output queue size. */
  size_t txLength;
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
#endif /* HALM_PLATFORM_LPC_SERIAL_H_ */
