/*
 * halm/generic/i2c.h
 * Copyright (C) 2022 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_GENERIC_I2C_H_
#define HALM_GENERIC_I2C_H_
/*----------------------------------------------------------------------------*/
#include <xcore/interface.h>
/*----------------------------------------------------------------------------*/
enum I2CParameter
{
  /**
   * Enable generation of the repeated start condition after a next transfer.
   * Data pointer should be set to zero.
   */
  IF_I2C_REPEATED_START = IF_PARAMETER_END,
  /**
   * Generate bus recovery sequence. Bus recovery is a blocking operation.
   * Data pointer should be set to zero.
   */
  IF_I2C_BUS_RECOVERY,
  /**
   * Enable 10-bit device address mode. Parameter type is \a uint8_t. Possible
   * values: 0 to disable 10-bit address mode and 1 to enable it. This mode
   * is disabled by default.
   */
  IF_I2C_10BIT_ADDRESS
};
/*----------------------------------------------------------------------------*/
#endif /* HALM_GENERIC_I2C_H_ */
