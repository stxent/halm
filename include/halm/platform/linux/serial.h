/*
 * halm/platform/linux/serial.h
 * Copyright (C) 2017 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef HALM_PLATFORM_LINUX_SERIAL_H_
#define HALM_PLATFORM_LINUX_SERIAL_H_
/*----------------------------------------------------------------------------*/
#include <stdint.h>
#include <xcore/interface.h>
/*----------------------------------------------------------------------------*/
enum SerialOption
{
  IF_SERIAL_CTS = IF_PARAMETER_END,
  IF_SERIAL_RTS
};

enum SerialParity
{
  SERIAL_PARITY_NONE,
  SERIAL_PARITY_ODD,
  SERIAL_PARITY_EVEN
};
/*----------------------------------------------------------------------------*/
extern const struct InterfaceClass * const Serial;

struct SerialConfig
{
  /** Mandatory: path to the character device. */
  const char *device;
  /** Mandatory: baud rate. */
  uint32_t rate;
  /** Optional: parity bit setting. */
  enum SerialParity parity;
};
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_LINUX_SERIAL_H_ */
