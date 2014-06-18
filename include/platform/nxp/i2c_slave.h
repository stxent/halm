/*
 * platform/nxp/i2c_slave.h
 * Copyright (C) 2014 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef I2C_SLAVE_H_
#define I2C_SLAVE_H_
/*----------------------------------------------------------------------------*/
#include <stdbool.h>
#include "i2c_base.h"
/*----------------------------------------------------------------------------*/
extern const struct InterfaceClass * const I2cSlave;
/*----------------------------------------------------------------------------*/
enum i2cSlaveState
{
  I2C_SLAVE_IDLE = 0,
  I2C_SLAVE_ADDRESS,
  I2C_SLAVE_DATA
};
/*----------------------------------------------------------------------------*/
struct I2cSlaveConfig
{
  /** Mandatory: register map size. */
  uint16_t size;
  /** Mandatory: serial clock pin. */
  gpio_t scl;
  /** Mandatory: serial data pin. */
  gpio_t sda;
  /** Optional: interrupt priority. */
  priority_t priority;
  /** Mandatory: peripheral identifier. */
  uint8_t channel;
};
/*----------------------------------------------------------------------------*/
struct I2cSlave
{
  struct I2cBase parent;

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
  enum i2cSlaveState state;
};
/*----------------------------------------------------------------------------*/
#endif /* I2C_SLAVE_H_ */
