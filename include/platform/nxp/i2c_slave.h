/*
 * platform/nxp/i2c_slave.h
 * Copyright (C) 2014 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef PLATFORM_NXP_I2C_SLAVE_H_
#define PLATFORM_NXP_I2C_SLAVE_H_
/*----------------------------------------------------------------------------*/
#include <stdbool.h>
#include <libhalm/target.h>
/*----------------------------------------------------------------------------*/
#undef HEADER_PATH
#define HEADER_PATH <platform/PLATFORM_TYPE/GEN_I2C/i2c_base.h>
#include HEADER_PATH
#undef HEADER_PATH
/*----------------------------------------------------------------------------*/
extern const struct InterfaceClass * const I2cSlave;
/*----------------------------------------------------------------------------*/
struct I2cSlaveConfig
{
  /** Mandatory: register map size. Map size is limited by 64 kB. */
  uint16_t size;
  /** Mandatory: serial clock pin. */
  pinNumber scl;
  /** Mandatory: serial data pin. */
  pinNumber sda;
  /** Optional: interrupt priority. */
  irqPriority priority;
  /** Mandatory: peripheral identifier. */
  uint8_t channel;
};
/*----------------------------------------------------------------------------*/
struct I2cSlave
{
  struct I2cBase base;

  void (*callback)(void *);
  void *callbackArgument;

  /* Register cache */
  uint8_t *cache;
  /* Cache size */
  uint16_t size;
  /* Position in register cache for external access */
  uint16_t external;
  /* Position in register cache for internal operations */
  uint16_t internal;

  /* Current interface state */
  uint8_t state;
};
/*----------------------------------------------------------------------------*/
#endif /* PLATFORM_NXP_I2C_SLAVE_H_ */
