/*
 * halm/platform/lpc/spifi.h
 * Copyright (C) 2023 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_LPC_SPIFI_H_
#define HALM_PLATFORM_LPC_SPIFI_H_
/*----------------------------------------------------------------------------*/
#include <halm/generic/spim.h>
#include <halm/platform/lpc/spifi_base.h>
#include <xcore/interface.h>
/*----------------------------------------------------------------------------*/
extern const struct InterfaceClass * const Spifi;

struct SpifiConfig
{
  /**
   * Optional: minimum chip select high time before a next command,
   * measured in serial clock periods. In case of zero value a maximum possible
   * delay will be used. When set, it should be in the range of 1 to 16.
   */
  uint32_t delay;
  /**
   * Optional: maximum chip select low time after a last memory access in
   * memory-mapped mode, measured in serial clock periods. In case of zero
   * value a maximum possible timeout will be used. When set, it should be
   * in the range of 1 to 65536.
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
  /** Mandatory: peripheral identifier. */
  uint8_t channel;
  /** Mandatory: mode number. */
  uint8_t mode;
  /** Mandatory: direct memory access channels. */
  uint8_t dma;
  /**
   * Optional: use large 128 MB memory-mapped region instead of 64 MB region
   * with debug capabilites.
   */
  bool large;
};

struct Spifi
{
  struct SpifiBase base;

  void (*callback)(void *);
  void *callbackArgument;

  /* DMA descriptor for RX transfers */
  struct Dma *rxDma;
  /* DMA descriptor for TX transfers */
  struct Dma *txDma;

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
    /* Post-address value */
    uint32_t value;
    /* Length of the post-address field */
    uint8_t length;
    /* Enable serial mode for the post-address field */
    bool serial;
  } post;

  struct
  {
    /* Number of data bytes */
    uint16_t length;
    /* Command response */
    uint8_t response;
    /* Enable serial mode for data */
    bool serial;
  } data;

  /* Operation status */
  uint8_t status;
  /* Enables blocking mode instead of zero-copy mode */
  bool blocking;
  /* Use large memory-mapped region instead of smaller debug-enabled region */
  bool large;
  /* Memory-mapping mode flag */
  bool memmap;
  /* Polling mode flag */
  bool poll;
  /* Synchronization of SPIFI and DMA IRQ handlers */
  bool sync;
};
/*----------------------------------------------------------------------------*/
BEGIN_DECLS

static inline void *spifiAddress(const struct Spifi *interface)
{
  return spifiGetMemoryAddress(&interface->base, interface->large);
}

END_DECLS
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_LPC_SPIFI_H_ */
