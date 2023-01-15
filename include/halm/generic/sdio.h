/*
 * halm/generic/sdio.h
 * Copyright (C) 2014 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_GENERIC_SDIO_H_
#define HALM_GENERIC_SDIO_H_
/*----------------------------------------------------------------------------*/
#include <xcore/interface.h>
/*----------------------------------------------------------------------------*/
enum SDIOFlags
{
  /** Send initialization sequence to device. */
  SDIO_INITIALIZE     = 0x01,
  /** Generate and check checksum. */
  SDIO_CHECK_CRC      = 0x02,
  /** Enable data transfer mode. */
  SDIO_DATA_MODE      = 0x04,
  /** Clear to read data from device, set to write data to device. */
  SDIO_WRITE_MODE     = 0x08,
  /** Stop the transfer when the data transfer is in progress. */
  SDIO_STOP_TRANSFER  = 0x10,
  /** Wait for a previous data transfer completion. */
  SDIO_WAIT_DATA      = 0x20,
  /** Send stop command at the end of the data transfer. */
  SDIO_AUTO_STOP      = 0x40
};

enum SDIOMode
{
  SDIO_SPI,
  SDIO_1BIT,
  SDIO_4BIT,
  SDIO_8BIT
} __attribute__((packed));

enum SDIOParameter
{
  /**
   * Interface connection type. Possible values are defined in the
   * \a enum \a SDIOMode. Parameter type is \a uint8_t.
   */
  IF_SDIO_MODE = IF_PARAMETER_END,
  /** Execute a prepared command. Data pointer should be set to zero. */
  IF_SDIO_EXECUTE,
  /** SDIO command argument. Parameter type is \a uint32_t. */
  IF_SDIO_ARGUMENT,
  /** SDIO command code. Parameter type is \a uint32_t. */
  IF_SDIO_COMMAND,
  /** SDIO response. Parameter type is an array of 4 \a uint32_t. */
  IF_SDIO_RESPONSE,
  /** Size of the single block. Parameter type is \a uint32_t. */
  IF_SDIO_BLOCK_SIZE
};

enum SDIOResponse
{
  SDIO_RESPONSE_NONE,
  SDIO_RESPONSE_SHORT,
  SDIO_RESPONSE_LONG
} __attribute__((packed));
/*----------------------------------------------------------------------------*/
#endif /* HALM_GENERIC_SDIO_H_ */
