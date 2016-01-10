/*
 * platform/sdio_spi.h
 * Copyright (C) 2014 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef PLATFORM_SDIO_SPI_H_
#define PLATFORM_SDIO_SPI_H_
/*----------------------------------------------------------------------------*/
#include <crc.h>
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
   * lower or equal priority than serial interface interrupts.
   */
  struct Timer *timer;
  /**
   * Optional: integrity checking control, set to zero to disable integrity
   * checks or to positive value to set maximal number of blocks in
   * single transfer.
   */
  uint16_t blocks;
  /** Mandatory: chip select pin. */
  pinNumber cs;
};
/*----------------------------------------------------------------------------*/
struct SdioSpi
{
  struct Interface base;

  void (*callback)(void *);
  void *callbackArgument;

  struct CrcEngine *crc7;
  struct CrcEngine *crc16;

  /* Parent SPI interface */
  struct Interface *bus;
  /* Timer to generate periodic interrupt requests */
  struct Timer *timer;

  /* Pool for results of checksum calculations */
  uint16_t *crcPool;
  /* Pool size */
  uint16_t crcPoolSize;
  /* Run the deferred verification of the received data */
  bool checkReceivedCrc;

  /* Pointer to an input buffer */
  uint8_t *rxBuffer;
  /* Pointer to an output buffer */
  const uint8_t *txBuffer;
  /* Number of bytes left */
  uint32_t left;
  /* Number of bytes to be sent or received */
  uint32_t length;
  /* Block size in data transfers */
  uint16_t blockSize;

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
  /*
   * Status of the high-level command, some commands may be split up into
   * several low-level commands.
   */
  enum result transferStatus;

  /* Pin connected to the chip select signal of the device */
  struct Pin cs;
};
/*----------------------------------------------------------------------------*/
#endif /* PLATFORM_SDIO_SPI_H_ */
