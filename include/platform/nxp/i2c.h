/*
 * platform/nxp/i2c.h
 * Copyright (C) 2013 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef I2C_H_
#define I2C_H_
/*----------------------------------------------------------------------------*/
#include <spinlock.h>
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
enum i2cOption
{
  IF_I2C_SENDSTOP = IF_END_OPTION /* Repeated start condition generation */
};
/*----------------------------------------------------------------------------*/
struct I2cConfig
{
  uint32_t rate; /* Mandatory: interface data rate */
  gpio_t scl, sda; /* Mandatory: interface pins */
  uint8_t channel; /* Mandatory: peripheral number */
};
/*----------------------------------------------------------------------------*/
struct I2c
{
  struct I2cBase parent;

  /* Pointer to the callback function and to the callback argument */
  void (*callback)(void *);
  void *callbackArgument;

  /* Pointer used in read mode */
  uint8_t *rxBuffer;
  /* Pointer used in write mode */
  const uint8_t *txBuffer;
  /* Bytes left for transmission or reception */
  uint16_t rxCount, txCount;

  /* Current interface state */
  volatile enum i2cState state;
  /* Exclusive access to the channel */
  spinlock_t lock;
  /* Address of the device, only 7-bit addressing supported */
  uint8_t address;
  /* Selection between blocking mode and zero copy mode */
  bool blocking;
  /* Generate stop condition after writing */
  bool stop;
};
/*----------------------------------------------------------------------------*/
#endif /* I2C_H_ */
