/*
 * halm/generic/spi.h
 * Copyright (C) 2018 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef HALM_GENERIC_SPI_H_
#define HALM_GENERIC_SPI_H_
/*----------------------------------------------------------------------------*/
#include <stdint.h>
#include <xcore/interface.h>
/*----------------------------------------------------------------------------*/
enum SpiParameter
{
  /**
   * Change interface mode, consisting of phase and clock settings,
   * or read the current settings.
   */
  IF_SPI_MODE = IF_PARAMETER_END,
  /**
   * Change word width settings or read current one.
   */
  IF_SPI_WIDTH,
  /**
   * Enable the bidirectional interface mode. Read request will be queued
   * until next write request.
   */
  IF_SPI_BIDIRECTIONAL,
  /**
   * Enable the unidirectional interface mode. This mode is used by default.
   */
  IF_SPI_UNIDIRECTIONAL
};
/*----------------------------------------------------------------------------*/
#endif /* HALM_GENERIC_SPI_H_ */
