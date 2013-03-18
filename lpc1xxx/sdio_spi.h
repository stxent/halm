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
#include "ssp.h"
/*----------------------------------------------------------------------------*/
extern const struct InterfaceClass *SdioSpi;
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
  struct Gpio csPin;
//  bool ready; //TODO Add hotswap support?
  bool highCapacity;
  Mutex lock;
};
/*----------------------------------------------------------------------------*/
#endif /* SDIO_SPI_H_ */
