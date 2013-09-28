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

  /* Pointer to the callback function and to the callback argument */
  void (*callback)(void *);
  void *callbackArgument;

  /* Pointers to input and output buffers */
  uint8_t *rxBuffer;
  const uint8_t *txBuffer;
  /* Bytes left for transmission or reception */
  uint16_t rxCount, txCount;

  /* Current interface state */
  enum i2cState state;
  /* Exclusive access to the channel */
  spinlock_t lock;
  /* Address of the device, only 7-bit addressing supported */
  uint8_t address;
  /* Selection between blocking mode and zero copy mode */
  bool blocking;
};
/*----------------------------------------------------------------------------*/
#endif /* I2C_H_ */
