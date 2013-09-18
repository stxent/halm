/*
 * i2c_base.h
 * Copyright (C) 2013 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef I2C_BASE_H_
#define I2C_BASE_H_
/*----------------------------------------------------------------------------*/
#include "interface.h"
#include "platform/gpio.h"
#include "./device_defs.h"
#include "./nvic.h"
/*----------------------------------------------------------------------------*/
extern const struct InterfaceClass *I2cBase;
/*----------------------------------------------------------------------------*/
/* TODO Add master/slave select */
struct I2cConfig
{
  uint32_t rate; /* Mandatory: data rate */
  gpioKey scl, sda; /* Mandatory: interface pins */
  uint8_t channel; /* Mandatory: peripheral number */
};
/*----------------------------------------------------------------------------*/
struct I2cBase
{
  struct Interface parent;

  void *reg; /* Pointer to I2C registers */
  irq_t irq; /* Interrupt identifier */

  void (*handler)(void *); /* Interrupt handler */
  struct Gpio sclPin, sdaPin;
  uint8_t channel; /* Peripheral number */
};
/*----------------------------------------------------------------------------*/
#endif /* I2C_BASE_H_ */
