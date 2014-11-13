/*
 * platform/nxp/serial_poll.h
 * Copyright (C) 2014 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef PLATFORM_NXP_SERIAL_POLL_H_
#define PLATFORM_NXP_SERIAL_POLL_H_
/*----------------------------------------------------------------------------*/
#include <mcu.h>
/*----------------------------------------------------------------------------*/
#undef HEADER_PATH
#define HEADER_PATH <platform/PLATFORM_TYPE/GENERATION/uart_base.h>
#include HEADER_PATH
/*----------------------------------------------------------------------------*/
extern const struct InterfaceClass * const SerialPoll;
/*----------------------------------------------------------------------------*/
struct SerialPollConfig
{
  /** Mandatory: baud rate. */
  uint32_t rate;
  /** Mandatory: serial input. */
  pin_t rx;
  /** Mandatory: serial output. */
  pin_t tx;
  /** Mandatory: peripheral identifier. */
  uint8_t channel;
  /** Optional: parity generation and checking. */
  enum uartParity parity;
};
/*----------------------------------------------------------------------------*/
struct SerialPoll
{
  struct UartBase parent;

  /* Desired baud rate */
  uint32_t rate;
};
/*----------------------------------------------------------------------------*/
#endif /* PLATFORM_NXP_SERIAL_POLL_H_ */
