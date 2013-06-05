/*
 * sdio_spi.h
 * Copyright (C) 2013 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef SDIO_SPI_H_
#define SDIO_SPI_H_
/*----------------------------------------------------------------------------*/
#include "interface.h"
#include "mutex.h"
#include "gpio.h"
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
  gpioKey cs;
};
/*----------------------------------------------------------------------------*/
struct SdioSpi
{
  struct Interface parent;

  struct Interface *interface;
  uint64_t position;
  uint32_t id; /* Unique device identifier */
  struct Gpio csPin;
  Mutex lock;
  enum cardType capacity;
};
/*----------------------------------------------------------------------------*/
#endif /* SDIO_SPI_H_ */
