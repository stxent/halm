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
  /** Enable parity checking or read current parity settings. */
  IF_SERIAL_PARITY = IF_PARAMETER_END,
  /** Read the CTS status line. */
  IF_SERIAL_CTS,
  /** Write the RTS status line. */
  IF_SERIAL_RTS,
  /** Read the DSR status line. */
  IF_SERIAL_DSR,
  /** Write the DTR status line. */
  IF_SERIAL_DTR
};
/*----------------------------------------------------------------------------*/
#endif /* HALM_GENERIC_SERIAL_H_ */
