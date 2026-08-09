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
  /** Set or read the parity checking settings. Parameter type is \p uint8_t. */
  IF_SERIAL_PARITY = IF_PARAMETER_END,

  /** Set or read the stop bit count. Parameter type is \p uint8_t. */
  IF_SERIAL_STOPBITS,

  /**
   * Read the CTS (Clear to Send) modem status line.
   * Parameter type is \p uint8_t. Valid values: 0 means line is inactive,
   * 1 means line is active.
   */
  IF_SERIAL_CTS,

  /**
   * Write the RTS (Request to Send) modem control line.
   * Parameter type is \p uint8_t. Valid values: 0 to deactivate the line,
   * 1 to activate the line.
   */
  IF_SERIAL_RTS,

  /**
   * Read the DSR (Data Set Ready) modem status line.
   * Parameter type is \p uint8_t. Valid values: 0 means DCE is not ready,
   * 1 means DCE is ready.
   */
  IF_SERIAL_DSR,

  /**
   * Write the DTR (Data Terminal Ready) modem control line.
   * Parameter type is \p uint8_t. Valid values: 0 to deactivate the DTR line,
   * 1 to activate the DTR line.
   */
  IF_SERIAL_DTR,

  /** Read the framing error counter. Parameter type is \p uint32_t. */
  IF_SERIAL_FE,

  /** Read the parity error counter. Parameter type is \p uint32_t. */
  IF_SERIAL_PE,

  /** End of the serial parameter list. */
  IF_SERIAL_PARAMETER_END
};
/*----------------------------------------------------------------------------*/
#endif /* HALM_GENERIC_SERIAL_H_ */
