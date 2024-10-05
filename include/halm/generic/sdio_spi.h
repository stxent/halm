/*
 * halm/generic/sdio_spi.h
 * Copyright (C) 2014 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_GENERIC_SDIO_SPI_H_
#define HALM_GENERIC_SDIO_SPI_H_
/*----------------------------------------------------------------------------*/
#include <xcore/interface.h>
#include <halm/pin.h>
/*----------------------------------------------------------------------------*/
extern const struct InterfaceClass * const SdioSpi;

struct Timer;
struct WorkQueue;

struct SdioSpiConfig
{
  /** Mandatory: serial interface. */
  void *interface;
  /**
   * Optional: timer to reduce interrupt count. Timer interrupts should have
   * lower or equal priority than serial interface interrupts.
   */
  void *timer;
  /** Optional: work queue for integrity checking tasks. */
  void *wq;
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

  struct
  {
    /* Pool for results of checksum computation */
    uint16_t *pool;
    /* Pool size */
    size_t capacity;
  } crc;

  struct
  {
    /* Address of an input or output buffer */
    uintptr_t buffer;
    /* Number of bytes left */
    size_t left;
    /* Total transfer size */
    size_t length;
    /*
     * Status of the high-level command, some commands may be split up into
     * multiple low-level commands.
     */
    uint8_t status;
  } transfer;

  struct
  {
    /* Argument for the most recent command */
    uint32_t argument;
    /* Command configuration */
    uint32_t code;
    /* Command response */
    uint32_t response[4];
    /* Buffer for temporary data, address should be aligned */
    uint8_t buffer[18];

    /* Status of the last command */
    uint8_t status;
  } command;

  /* Bus rate */
  uint32_t rate;
  /* Retries for a last command or transfer step */
  unsigned short retries;
  /* Size of each block in multi-block data transfers */
  uint16_t block;
  /* Current state of the FSM */
  uint8_t state;

  /* Pin connected to the chip select signal of the device */
  struct Pin cs;
};
/*----------------------------------------------------------------------------*/
#endif /* HALM_GENERIC_SDIO_SPI_H_ */
