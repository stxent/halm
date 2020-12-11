/*
 * halm/platform/lpc/serial_poll.h
 * Copyright (C) 2014 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_LPC_SERIAL_POLL_H_
#define HALM_PLATFORM_LPC_SERIAL_POLL_H_
/*----------------------------------------------------------------------------*/
#include <halm/platform/lpc/uart_base.h>
/*----------------------------------------------------------------------------*/
extern const struct InterfaceClass * const SerialPoll;

struct SerialPollConfig
{
  /** Mandatory: baud rate. */
  uint32_t rate;
  /** Optional: parity bit setting. */
  enum SerialParity parity;
  /** Mandatory: serial input. */
  PinNumber rx;
  /** Mandatory: serial output. */
  PinNumber tx;
  /** Mandatory: peripheral identifier. */
  uint8_t channel;
};

struct SerialPoll
{
  struct UartBase base;

  /* Desired baud rate */
  uint32_t rate;
};
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_LPC_SERIAL_POLL_H_ */
