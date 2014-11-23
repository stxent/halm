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
#endif /* PLATFORM_NXP_SDIO_SPI_H_ */
