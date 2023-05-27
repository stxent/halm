/*
 * halm/platform/lpc/i2c_slave.h
 * Copyright (C) 2014 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_LPC_I2C_SLAVE_H_
#define HALM_PLATFORM_LPC_I2C_SLAVE_H_
/*----------------------------------------------------------------------------*/
#include <halm/platform/lpc/i2c_base.h>
#include <stdbool.h>
/*----------------------------------------------------------------------------*/
extern const struct InterfaceClass * const I2CSlave;

struct I2CSlaveConfig
{
  /** Mandatory: register map size. Map size is limited to 256 bytes. */
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
  uint8_t size;
  /* Position in register cache for external access */
  uint8_t external;
  /* Position in register cache for program access */
  uint8_t internal;

  /* Current interface state */
  uint8_t state;
};
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_LPC_I2C_SLAVE_H_ */
