/*
 * halm/platform/numicro/spim.h
 * Copyright (C) 2023 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_NUMICRO_SPIM_H_
#define HALM_PLATFORM_NUMICRO_SPIM_H_
/*----------------------------------------------------------------------------*/
#include <halm/generic/spim.h>
#include <halm/platform/numicro/spim_base.h>
#include <xcore/interface.h>
/*----------------------------------------------------------------------------*/
extern const struct InterfaceClass * const Spim;

struct Timer;

struct SpimConfig
{
  /**
   * Mandatory: timer for periodic polling of busy flag during
   * erase and write operations.
   */
  void *timer;
  /**
   * Optional: minimum chip select high time before a next command,
   * measured in serial clock periods. In case of zero value a maximum possible
   * delay will be used. If set, it should be in the range from 1 to 16.
   */
  uint32_t delay;
  /**
   * Optional: poll rate. If set to zero, a default poll rate of 100 Hz
   * will be used. Poll rate can't be higher than timer tick rate.
   */
  uint32_t poll;
  /**
   * Mandatory: serial data rate. If set to zero, serial data rate will
   * be the same as the AHB clock frequency. For DDR mode the serial data rate
   * must be two times lower than the AHB clock frequency.
   */
  uint32_t rate;
  /**
   * Optional: maximum chip select low time after a last memory access in
   * memory-mapped mode, measured in serial clock periods. In case of zero
   * value a maximum possible timeout will be used. If set, it should be
   * in the range from 1 to 32.
   */
  uint32_t timeout;
  /** Mandatory: chip select pin. */
  PinNumber cs;
  /**
   * Mandatory: data output in single-wire mode, input/output pin 0
   * in dual or quad mode.
   */
  PinNumber io0;
  /**
   * Mandatory: data input in single-wire mode, input/output pin 1
   * in dual or quad mode.
   */
  PinNumber io1;
  /** Optional: input/output pin 2 in quad mode. */
  PinNumber io2;
  /** Optional: input/output pin 3 in quad mode. */
  PinNumber io3;
  /** Mandatory: serial clock output. */
  PinNumber sck;
  /** Optional: interrupt priority. */
  IrqPriority priority;
  /** Mandatory: peripheral identifier. */
  uint8_t channel;
  /** Optional: disable cache function. */
  bool uncached;
};

struct Spim
{
  struct SpimBase base;

  void (*callback)(void *);
  void *callbackArgument;

  /* Timer for periodic polling of busy flag during erase and write commands */
  struct Timer *timer;

  struct
  {
    /* Requested address */
    uint32_t value;
    /* Length of the address field */
    uint8_t length;
    /* Enable serial mode for address field */
    bool serial;
  } address;

  struct
  {
    /* Length of the command field */
    uint8_t length;
    /* Requested command code */
    uint8_t value;
    /* Enable serial mode for command field */
    bool serial;
  } command;

  struct
  {
    /* Number of dummy bytes */
    uint8_t length;
    /* Enable serial mode for dummy bytes */
    bool serial;
  } delay;

  struct
  {
    /* Length of the post-address field */
    uint8_t length;
    /* Post-address value */
    uint8_t value;
    /* Enable serial mode for the post-address field */
    bool serial;
  } post;

  struct
  {
    /* Number of data bytes */
    uint32_t length;
    /* Command response */
    uint8_t response;
    /* Enable serial mode for data */
    bool serial;
  } data;

  /* Operation status */
  uint8_t status;
  /* Enables blocking mode instead of zero-copy mode */
  bool blocking;
  /* Enables DDR mode */
  bool ddr;
  /* Memory-mapping mode flag */
  bool memmap;
  /* Polling mode flag */
  bool poll;

  /* Enable Quad I/O mode */
  bool quad;
  /* Disable cache function */
  bool uncached;
};
/*----------------------------------------------------------------------------*/
BEGIN_DECLS

static inline void *spimGetAddress(const void *object)
{
  const struct Spim * const interface = object;
  return spimGetMemoryAddress(&interface->base);
}

END_DECLS
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_NUMICRO_SPIM_H_ */
