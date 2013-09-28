/*
 * i2c.h
 * Copyright (C) 2013 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef I2C_H_
#define I2C_H_
/*----------------------------------------------------------------------------*/
#include "platform/nxp/spinlock.h"
#include "./i2c_base.h"
/*----------------------------------------------------------------------------*/
extern const struct InterfaceClass *I2c;
/*----------------------------------------------------------------------------*/
enum i2cState
{
  I2C_IDLE = 0,
  I2C_ADDRESS,
  I2C_TRANSMIT,
  I2C_RECEIVE,
  I2C_ERROR
};
/*----------------------------------------------------------------------------*/
struct I2c
{
  struct I2cBase parent;

  void (*callback)(void *);
  void *callbackArgument;

  uint8_t *rxBuffer;
  const uint8_t *txBuffer;
  uint16_t rxCount, txCount;

  uint16_t address; /* For 7-bit and 11-bit addresses */
  bool blocking; /* By default interface is in blocking mode */
  enum i2cState state;
  spinlock_t lock;
};
/*----------------------------------------------------------------------------*/
#endif /* I2C_H_ */
