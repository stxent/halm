/*
 * platform/nxp/i2c_base.h
 * Copyright (C) 2013 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef I2C_BASE_H_
#define I2C_BASE_H_
/*----------------------------------------------------------------------------*/
#include <gpio.h>
#include <interface.h>
#include <irq.h>
/*----------------------------------------------------------------------------*/
extern const struct EntityClass *I2cBase;
/*----------------------------------------------------------------------------*/
struct I2cBaseConfig
{
  gpio_t scl, sda; /* Mandatory: interface pins */
  uint8_t channel; /* Mandatory: peripheral identifier */
};
/*----------------------------------------------------------------------------*/
struct I2cBase
{
  struct Interface parent;

  void *reg;
  void (*handler)(void *);
  irq_t irq;

  /* Unique peripheral identifier */
  uint8_t channel;
};
/*----------------------------------------------------------------------------*/
uint32_t i2cGetRate(struct I2cBase *);
void i2cSetRate(struct I2cBase *, uint32_t);

uint32_t i2cGetClock(struct I2cBase *);
enum result i2cSetupPins(struct I2cBase *, const struct I2cBaseConfig *);
/*----------------------------------------------------------------------------*/
#endif /* I2C_BASE_H_ */
