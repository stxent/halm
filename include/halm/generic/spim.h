/*
 * halm/generic/spim.h
 * Copyright (C) 2023 xent
 * Project is distributed under the terms of the MIT License
 */

/**
 * @file
 * Abstract class for SPI NAND and NOR Flash with QSPI interfaces.
 */

#ifndef HALM_GENERIC_SPIM_H_
#define HALM_GENERIC_SPIM_H_
/*----------------------------------------------------------------------------*/
#include <halm/generic/qspi.h>
/*----------------------------------------------------------------------------*/
enum SPIMParameter
{
  /**
   * SPI mode, consisting of phase and clock settings.
   * Possible values are 0 to 3. Parameter type is \a uint8_t.
   */
  IF_SPIM_MODE = IF_QSPI_MODE,
  /** Enable dual I/O mode. */
  IF_SPIM_DUAL = IF_QSPI_DUAL,
  /** Enable quad I/O mode. */
  IF_SPIM_QUAD = IF_QSPI_QUAD,
  /** Enable octal I/O mode for two parallel QSPI devices. */
  IF_SPIM_OCTAL = IF_QSPI_OCTAL,

  /** Enable standard single data rate mode. */
  IF_SPIM_SDR = IF_QSPI_SDR,
  /** Enable double data rate mode. */
  IF_SPIM_DDR = IF_QSPI_DDR,

  /** Enable indirect operation mode. */
  IF_SPIM_INDIRECT = IF_QSPI_PARAMETER_END,
  /** Enable memory-mapped mode. */
  IF_SPIM_MEMORY_MAPPED,

  /** Operation code. Parameter type is \a uint8_t. */
  IF_SPIM_COMMAND,
  /** Disable operation code phase. */
  IF_SPIM_COMMAND_NONE,
  /** Enable parallel mode for operation code phase. */
  IF_SPIM_COMMAND_PARALLEL,
  /** Enable serial mode for operation code phase. */
  IF_SPIM_COMMAND_SERIAL,

  /** Number of delay or dummy bytes. Parameter type is \a uint8_t. */
  IF_SPIM_DELAY_LENGTH,
  /** Disable dummy-cycles phase. */
  IF_SPIM_DELAY_NONE,
  /** Enable parallel mode for dummy-cycles phase. */
  IF_SPIM_DELAY_PARALLEL,
  /** Enable serial mode for dummy-cycles phase. */
  IF_SPIM_DELAY_SERIAL,

  /** Set 24-bit memory address. Parameter type is \a uint32_t. */
  IF_SPIM_ADDRESS_24,
  /** Set 32-bit memory address. Parameter type is \a uint32_t. */
  IF_SPIM_ADDRESS_32,
  /** Disable address field phase. */
  IF_SPIM_ADDRESS_NONE,
  /** Enable parallel mode for address phase. */
  IF_SPIM_ADDRESS_PARALLEL,
  /** Enable serial mode for address phase. */
  IF_SPIM_ADDRESS_SERIAL,

  /** Set 8-bit post-address data. Parameter type is \a uint32_t. */
  IF_SPIM_POST_ADDRESS_8,
  /** Disable post-address field phase. */
  IF_SPIM_POST_ADDRESS_NONE,
  /** Enable parallel mode for post-address phase. */
  IF_SPIM_POST_ADDRESS_PARALLEL,
  /** Enable serial mode for post-address phase. */
  IF_SPIM_POST_ADDRESS_SERIAL,

  /** Number of bytes read or written in data phase. */
  IF_SPIM_DATA_LENGTH,
  /** Disable data phase. */
  IF_SPIM_DATA_NONE,
  /**
   * Set an index of a polling bit and enable the poll mode for a next command.
   * Hardware polls the memory until this bit becomes cleared.
   * Parameter type is \a uint8_t.
   */
  IF_SPIM_DATA_POLL_BIT,
  /** Enable parallel mode for data phase. */
  IF_SPIM_DATA_PARALLEL,
  /** Enable serial mode for data phase. */
  IF_SPIM_DATA_SERIAL,
  /** Command response. Parameter type is \a uint32_t. */
  IF_SPIM_RESPONSE,

  /** End of the list. */
  IF_SPIM_PARAMETER_END
};
/*----------------------------------------------------------------------------*/
#endif /* HALM_GENERIC_SPIM_H_ */
