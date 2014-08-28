/*
 * platform/sdio_spi.h
 * Copyright (C) 2013 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef PLATFORM_SDIO_SPI_H_
#define PLATFORM_SDIO_SPI_H_
/*----------------------------------------------------------------------------*/
#include <interface.h>
#include <pin.h>
/*----------------------------------------------------------------------------*/
extern const struct InterfaceClass * const SdioSpi;
/*----------------------------------------------------------------------------*/
enum cardType
{
  CARD_SD,
  CARD_SDHC,
  CARD_SDXC
};
/*----------------------------------------------------------------------------*/
struct SdioSpiConfig
{
  struct Interface *interface; /* Mandatory: low-level character device */
  pin_t cs; /* Mandatory: chip select pin */
};
/*----------------------------------------------------------------------------*/
struct SdioSpi
{
  struct Interface parent;

  /* Parent SPI interface */
  struct Interface *interface;
  /* Current position in internal memory space */
  uint64_t position;
  /* Pin connected to the chip select signal of card */
  struct Pin csPin;
  /* Type of the memory card */
  enum cardType capacity;
};
/*----------------------------------------------------------------------------*/
#endif /* PLATFORM_SDIO_SPI_H_ */
