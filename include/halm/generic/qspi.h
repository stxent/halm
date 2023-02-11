/*
 * halm/generic/qspi.h
 * Copyright (C) 2023 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_GENERIC_QSPI_H_
#define HALM_GENERIC_QSPI_H_
/*----------------------------------------------------------------------------*/
#include <halm/generic/spi.h>
/*----------------------------------------------------------------------------*/
enum QSPIParameter
{
  /**
   * Change interface mode, consisting of phase and clock settings,
   * or read the current settings. Possible values are 0 to 3.
   * Parameter type is \a uint8_t.
   */
  IF_QSPI_MODE = IF_SPI_MODE,

  /** Enable standard 3-wire mode. */
  IF_QSPI_SERIAL = IF_SPI_PARAMETER_END,
  /** Enable dual I/O mode. */
  IF_QSPI_DUAL,
  /** Enable quad I/O mode. */
  IF_QSPI_QUAD,
  /** Enable standard single data rate mode. */
  IF_QSPI_SDR,
  /** Enable double data rate mode. */
  IF_QSPI_DDR,

  /** End of the list. */
  IF_QSPI_PARAMETER_END
};
/*----------------------------------------------------------------------------*/
#endif /* HALM_GENERIC_QSPI_H_ */
