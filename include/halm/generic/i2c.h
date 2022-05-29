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
   */
  IF_I2C_REPEATED_START = IF_PARAMETER_END,
};
/*----------------------------------------------------------------------------*/
#endif /* HALM_GENERIC_I2C_H_ */
