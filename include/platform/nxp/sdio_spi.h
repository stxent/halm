/*
 * platform/nxp/sdio_spi.h
 * Copyright (C) 2013 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef SDIO_SPI_H_
#define SDIO_SPI_H_
/*----------------------------------------------------------------------------*/
#include <gpio.h>
#include <interface.h>
/*----------------------------------------------------------------------------*/
extern const struct InterfaceClass * const SdioSpi;
/*----------------------------------------------------------------------------*/
enum cardType
{
  CARD_SD = 0,
  CARD_SDHC,
  CARD_SDXC
};
/*----------------------------------------------------------------------------*/
struct SdioSpiConfig
{
  struct Interface *interface; /* Mandatory: low-level character device */
  gpio_t cs; /* Mandatory: chip select pin */
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
  struct Gpio csPin;
  /* Type of the memory card */
  enum cardType capacity;
};
/*----------------------------------------------------------------------------*/
#endif /* SDIO_SPI_H_ */
