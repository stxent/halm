/*
 * halm/generic/spi.h
 * Copyright (C) 2018 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_GENERIC_SPI_H_
#define HALM_GENERIC_SPI_H_
/*----------------------------------------------------------------------------*/
#include <xcore/interface.h>
/*----------------------------------------------------------------------------*/
enum SPIParameter
{
  /**
   * Change interface mode, consisting of phase and clock settings,
   * or read the current settings. Possible values are 0 to 3.
   * Parameter type is \a uint8_t.
   */
  IF_SPI_MODE = IF_PARAMETER_END,
  /**
   * Change word width of the interface or read the current settings.
   * Parameter type is \a uint8_t.
   */
  IF_SPI_WIDTH,
  /**
   * Enable the bidirectional interface mode. Read request will be queued
   * until next write request. Data pointer should be set to zero.
   */
  IF_SPI_BIDIRECTIONAL,
  /**
   * Enable the unidirectional interface mode. This mode is used by default.
   * Data pointer should be set to zero.
   */
  IF_SPI_UNIDIRECTIONAL
};
/*----------------------------------------------------------------------------*/
#endif /* HALM_GENERIC_SPI_H_ */
