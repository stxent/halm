/*
 * halm/platform/nxp/serial_poll.h
 * Copyright (C) 2014 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef HALM_PLATFORM_NXP_SERIAL_POLL_H_
#define HALM_PLATFORM_NXP_SERIAL_POLL_H_
/*----------------------------------------------------------------------------*/
#include <halm/target.h>
/*----------------------------------------------------------------------------*/
#undef HEADER_PATH
#define HEADER_PATH <halm/platform/PLATFORM_TYPE/GEN_UART/uart_base.h>
#include HEADER_PATH
#undef HEADER_PATH
/*----------------------------------------------------------------------------*/
extern const struct InterfaceClass * const SerialPoll;
/*----------------------------------------------------------------------------*/
struct SerialPollConfig
{
  /** Mandatory: baud rate. */
  uint32_t rate;
  /** Optional: parity generation and checking. */
  enum UartParity parity;
  /** Mandatory: serial input. */
  PinNumber rx;
  /** Mandatory: serial output. */
  PinNumber tx;
  /** Mandatory: peripheral identifier. */
  uint8_t channel;
};
/*----------------------------------------------------------------------------*/
struct SerialPoll
{
  struct UartBase base;

  /* Desired baud rate */
  uint32_t rate;
};
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_NXP_SERIAL_POLL_H_ */
