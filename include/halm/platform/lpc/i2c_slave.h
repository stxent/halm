/*
 * halm/platform/lpc/i2c_slave.h
 * Copyright (C) 2014 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef HALM_PLATFORM_LPC_I2C_SLAVE_H_
#define HALM_PLATFORM_LPC_I2C_SLAVE_H_
/*----------------------------------------------------------------------------*/
#include <halm/target.h>
#include <stdbool.h>
/*----------------------------------------------------------------------------*/
#undef HEADER_PATH
#define HEADER_PATH <halm/platform/PLATFORM_TYPE/GEN_I2C/i2c_base.h>
#include HEADER_PATH
#undef HEADER_PATH
/*----------------------------------------------------------------------------*/
extern const struct InterfaceClass * const I2CSlave;

struct I2CSlaveConfig
{
  /** Mandatory: register map size. Map size is limited by 64 kB. */
  size_t size;
  /** Mandatory: serial clock pin. */
  PinNumber scl;
  /** Mandatory: serial data pin. */
  PinNumber sda;
  /** Optional: interrupt priority. */
  IrqPriority priority;
  /** Mandatory: peripheral identifier. */
  uint8_t channel;
};

struct I2CSlave
{
  struct I2CBase base;

  void (*callback)(void *);
  void *callbackArgument;

  /* Register cache */
  uint8_t *cache;
  /* Cache size */
  uint16_t size;
  /* Position in register cache for external access */
  uint16_t external;
  /* Position in register cache for program access */
  uint16_t internal;

  /* Current interface state */
  uint8_t state;
};
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_LPC_I2C_SLAVE_H_ */
