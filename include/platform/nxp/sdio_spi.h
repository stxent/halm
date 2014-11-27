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
#include <timer.h>
/*----------------------------------------------------------------------------*/
extern const struct InterfaceClass * const SdioSpi;
/*----------------------------------------------------------------------------*/
struct SdioSpiConfig
{
  /** Mandatory: underlying serial interface. */
  struct Interface *interface;
  /**
   * Optional: timer to reduce interrupt count. Timer interrupts should have
   * lower priority than serial interface interrupts.
   */
  struct Timer *timer;
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
  struct Interface *bus;
  /* Timer to generate periodic interrupt requests */
  struct Timer *timer;

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
  /* Retries left */
  uint16_t retries;
  /* Buffer for temporary data */
  uint8_t buffer[18];
  /* Current interface state */
  uint8_t state;
  /* Status of the last command */
  enum result status;

  /* Pin connected to the chip select signal of the device */
  struct Pin cs;
};
/*----------------------------------------------------------------------------*/
#endif /* PLATFORM_NXP_SDIO_SPI_H_ */
