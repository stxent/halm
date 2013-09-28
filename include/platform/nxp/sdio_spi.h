/*
 * sdio_spi.h
 * Copyright (C) 2013 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef SDIO_SPI_H_
#define SDIO_SPI_H_
/*----------------------------------------------------------------------------*/
#include "interface.h"
#include "platform/gpio.h"
/*----------------------------------------------------------------------------*/
extern const struct InterfaceClass *SdioSpi;
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
  gpioKey cs; /* Mandatory: chip select pin */
};
/*----------------------------------------------------------------------------*/
struct SdioSpi
{
  struct Interface parent;

  void (*callback)(void *);
  void *callbackArgument;

  struct Interface *interface;
  uint64_t position;

  struct Gpio csPin;

  bool blocking; /* By default interface is in blocking mode */
  enum cardType capacity;
};
/*----------------------------------------------------------------------------*/
#endif /* SDIO_SPI_H_ */
