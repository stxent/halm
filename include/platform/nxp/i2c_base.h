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

  /* Pointer to the I2C register block */
  void *reg;
  /* Pointer to the interrupt handler */
  void (*handler)(void *);
  /* Interrupt identifier */
  irq_t irq;

  /* Interface pins */
  struct Gpio sclPin, sdaPin;
  /* Peripheral block identifier */
  uint8_t channel;
};
/*----------------------------------------------------------------------------*/
uint32_t i2cGetRate(const struct I2cBase *);
void i2cSetRate(struct I2cBase *, uint32_t);
/*----------------------------------------------------------------------------*/
#endif /* I2C_BASE_H_ */
