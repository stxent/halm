/*
 * halm/generic/sdio_spi.h
 * Copyright (C) 2014 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef HALM_GENERIC_SDIO_SPI_H_
#define HALM_GENERIC_SDIO_SPI_H_
/*----------------------------------------------------------------------------*/
#include <xcore/interface.h>
#include <halm/pin.h>
#include <halm/timer.h>
#include <halm/wq.h>
/*----------------------------------------------------------------------------*/
extern const struct InterfaceClass * const SdioSpi;

struct SdioSpiConfig
{
  /** Mandatory: underlying serial interface. */
  struct Interface *interface;
  /**
   * Optional: timer to reduce interrupt count. Timer interrupts should have
   * lower or equal priority than serial interface interrupts.
   */
  struct Timer *timer;
  /** Optional: work queue for integrity checking tasks. */
  struct WorkQueue *wq;
  /**
   * Optional: integrity checking control, set to zero to disable integrity
   * checks or to positive value to set maximal number of blocks in
   * single transfer.
   */
  size_t blocks;
  /** Mandatory: chip select pin. */
  PinNumber cs;
};

struct SdioSpi
{
  struct Interface base;

  void (*callback)(void *);
  void *callbackArgument;

  /* Base SPI interface */
  struct Interface *bus;
  /* Timer to generate periodic interrupt requests */
  struct Timer *timer;
  /* Work queue for integrity checking tasks */
  struct WorkQueue *wq;

  /* Pool for results of checksum computation */
  uint16_t *crcPool;
  /* Pool size */
  size_t crcPoolSize;

  /* Pointer to an input buffer */
  uint8_t *rxBuffer;
  /* Pointer to an output buffer */
  const uint8_t *txBuffer;
  /* Block size of data transfers */
  uint32_t blockSize;
  /* Number of bytes left */
  size_t left;
  /* Total transfer size */
  size_t length;

  /* Argument for the most recent command */
  uint32_t argument;
  /* Interface command */
  uint32_t command;
  /* Command response */
  uint32_t response[4];
  /* Buffer for temporary data */
  uint8_t buffer[18];

  /* Retries left */
  unsigned short retries;

  /* Current state of the FSM */
  uint8_t state;
  /* Status of the last command */
  uint8_t commandStatus;
  /*
   * Status of the high-level command, some commands may be split up into
   * several low-level commands.
   */
  uint8_t transferStatus;

  /* Pin connected to the chip select signal of the device */
  struct Pin cs;
};
/*----------------------------------------------------------------------------*/
#endif /* HALM_GENERIC_SDIO_SPI_H_ */
