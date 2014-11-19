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
  IF_SDIO_EXECUTE = IF_END_OPTION,
  IF_SDIO_ARGUMENT,
  IF_SDIO_COMMAND,
  IF_SDIO_RESPONSE
};
/*----------------------------------------------------------------------------*/
struct SdioSpiConfig
{
  /** Mandatory: underlying serial interface. */
  struct Interface *interface;
};
/*----------------------------------------------------------------------------*/
struct SdioSpi
{
  struct Interface parent;

  /* Parent SPI interface */
  struct Interface *interface;
  /* Argument for the most recent command */
  uint32_t argument;
  /* Interface command */
  uint32_t command;
  /* Command response */
  uint32_t response[4];
};
/*----------------------------------------------------------------------------*/
enum sdioResponseType
{
  SDIO_RESPONSE_NONE,
  SDIO_RESPONSE_SHORT,
  SDIO_RESPONSE_LONG
};
/*----------------------------------------------------------------------------*/
enum sdioDataMode
{
  SDIO_DATA_NONE,
  SDIO_DATA_READ,
  SDIO_DATA_WRITE
};
/*----------------------------------------------------------------------------*/
enum sdioFlags
{
  SDIO_BLOCK_MODE = 0x01,
  SDIO_CHECK_CRC  = 0x02,
  SDIO_INITIALIZE = 0x04
};
/*----------------------------------------------------------------------------*/
uint32_t sdioPrepareCommand(uint8_t, enum sdioDataMode, enum sdioResponseType,
    uint8_t);
/*----------------------------------------------------------------------------*/
#endif /* PLATFORM_NXP_SDIO_SPI_H_ */
