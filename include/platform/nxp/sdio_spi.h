/*
 * platform/nxp/sdio_spi.h
 * Copyright (C) 2014 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef PLATFORM_NXP_SDIO_SPI_H_
#define PLATFORM_NXP_SDIO_SPI_H_
/*----------------------------------------------------------------------------*/
#include <interface.h>
#include <pin.h>
/*----------------------------------------------------------------------------*/
extern const struct InterfaceClass * const SdioSpi;
/*----------------------------------------------------------------------------*/
enum sdioOption
{
  IF_SDIO_MODE = IF_END_OPTION,
  IF_SDIO_EXECUTE,
  IF_SDIO_ARGUMENT,
  IF_SDIO_COMMAND,
  IF_SDIO_RESPONSE,
};
/*----------------------------------------------------------------------------*/
enum sdioMode
{
  SDIO_SPI,
  SDIO_1BIT,
  SDIO_4BIT
};
/*----------------------------------------------------------------------------*/
struct SdioSpiConfig
{
  /** Mandatory: underlying serial interface. */
  struct Interface *interface;
  /** Mandatory: chip select pin. */
  pin_t cs;
};
/*----------------------------------------------------------------------------*/
struct SdioSpi
{
  struct Interface parent;

  /* Parent SPI interface */
  struct Interface *interface;
  /* Pin connected to the chip select signal of the device */
  struct Pin cs;
  /* Argument for the most recent command */
  uint32_t argument;
  /* Interface command */
  uint32_t command;
  /* Command response */
  uint32_t response[4];
  /* Buffer for temporary data */
  uint8_t buffer[16];
  /* Status of the last command */
  enum result status;
};
/*----------------------------------------------------------------------------*/
enum sdioResponse
{
  SDIO_RESPONSE_NONE,
  SDIO_RESPONSE_SHORT,
  SDIO_RESPONSE_LONG
};
/*----------------------------------------------------------------------------*/
enum sdioFlags
{
  /** Send initialization sequence to device. */
  SDIO_INITIALIZE   = 0x01,
  /** Generate and check checksum. */
  SDIO_CHECK_CRC    = 0x02,
  /** Enable data transfer mode. */
  SDIO_DATA_MODE    = 0x04,
  /** Clear to read data from device, set to write data to device. */
  SDIO_READ_WRITE   = 0x08,
  /** Enable stream transfer mode instead of block mode. */
  SDIO_STREAM_MODE  = 0x10,
  /** Send stop command at the end of data transfer. */
  SDIO_AUTO_STOP    = 0x20
};
/*----------------------------------------------------------------------------*/
uint32_t sdioPrepareCommand(uint8_t, enum sdioResponse, uint16_t);
/*----------------------------------------------------------------------------*/
#endif /* PLATFORM_NXP_SDIO_SPI_H_ */
