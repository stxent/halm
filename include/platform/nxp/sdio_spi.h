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

  void (*callback)(void *);
  void *callbackArgument;

  /* Parent SPI interface */
  struct Interface *interface;

  /* Pointer to an input buffer */
  uint8_t *rxBuffer;
  /* Pointer to an output buffer */
  const uint8_t *txBuffer;
  /* Number of bytes to be sent or received */
  uint32_t left;
  /* Block length in data transfers */
  uint16_t blockLength;

  /* Argument for the most recent command */
  uint32_t argument;
  /* Interface command */
  uint32_t command;
  /* Command response */
  uint32_t response[4];
  /* Buffer for temporary data */
  uint8_t buffer[18];
  /* Iteration */
  uint8_t iteration;
  /* Current interface state */
  uint8_t state;
  /* Status of the last command */
  enum result status;
  /* Status of the last processed token */
  enum result tokenStatus;

  /* Pin connected to the chip select signal of the device */
  struct Pin cs;
  /* Selection between blocking mode and zero copy mode */
  bool blocking;
};
/*----------------------------------------------------------------------------*/
#endif /* PLATFORM_NXP_SDIO_SPI_H_ */
