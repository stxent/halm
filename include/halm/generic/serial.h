/*
 * halm/generic/serial.h
 * Copyright (C) 2018 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef HALM_GENERIC_SERIAL_H_
#define HALM_GENERIC_SERIAL_H_
/*----------------------------------------------------------------------------*/
#include <stdint.h>
#include <xcore/interface.h>
/*----------------------------------------------------------------------------*/
enum SerialParity
{
  SERIAL_PARITY_NONE,
  SERIAL_PARITY_ODD,
  SERIAL_PARITY_EVEN
};

enum SerialParameter
{
  /** Access parity settings. */
  IF_SERIAL_PARITY = IF_PARAMETER_END,
  /** Controls the Clear to Send signal. */
  IF_SERIAL_CTS,
  /** Controls the Request to Send signal. */
  IF_SERIAL_RTS
};
/*----------------------------------------------------------------------------*/
#endif /* HALM_GENERIC_SERIAL_H_ */
