/*
 * halm/generic/serial.h
 * Copyright (C) 2018 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_GENERIC_SERIAL_H_
#define HALM_GENERIC_SERIAL_H_
/*----------------------------------------------------------------------------*/
#include <xcore/interface.h>
/*----------------------------------------------------------------------------*/
enum [[gnu::packed]] SerialParity
{
  SERIAL_PARITY_NONE,
  SERIAL_PARITY_ODD,
  SERIAL_PARITY_EVEN
};

enum [[gnu::packed]] SerialStopBits
{
  SERIAL_STOPBITS_1,
  SERIAL_STOPBITS_2,
  SERIAL_STOPBITS_0P5,
  SERIAL_STOPBITS_1P5
};

enum SerialParameter
{
  /**
   * Enable parity checking or read current parity settings.
   * Parameter type is \a uint8_t.
   */
  IF_SERIAL_PARITY = IF_PARAMETER_END,

  /**
   * Set stop bit count for the transmitter or read current stop bit settings.
   * Parameter type is \a uint8_t.
   */
  IF_SERIAL_STOPBITS,

  /**
   * Read the CTS status line. Parameter type is \a uint8_t. Possible values:
   * 0 means carrier is deactivated, 1 means carrier is activated.
   */
  IF_SERIAL_CTS,
  /**
   * Write the RTS status line. Parameter type is \a uint8_t.
   * Possible values: 0 to deactivate carrier and 1 to activate carrier.
   */
  IF_SERIAL_RTS,
  /**
   * Read the DSR status line. Parameter type is \a uint8_t. Possible values:
   * 0 means DTE not present, 1 means DTE is present.
   */
  IF_SERIAL_DSR,
  /**
   * Write the DTR status line. Parameter type is \a uint8_t.
   * Possible values: 0 for DTE not present and 1 for DTE present.
   */
  IF_SERIAL_DTR,

  /** Read frame error counter. Parameter type is \a uint32_t. */
  IF_SERIAL_FE,
  /** Read parity error counter. Parameter type is \a uint32_t. */
  IF_SERIAL_PE,

  /** End of the list. */
  IF_SERIAL_PARAMETER_END
};
/*----------------------------------------------------------------------------*/
#endif /* HALM_GENERIC_SERIAL_H_ */
