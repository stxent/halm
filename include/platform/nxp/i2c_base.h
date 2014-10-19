/*
 * platform/nxp/i2c_base.h
 * Copyright (C) 2013 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef PLATFORM_NXP_I2C_BASE_H_
#define PLATFORM_NXP_I2C_BASE_H_
/*----------------------------------------------------------------------------*/
#include <interface.h>
#include <irq.h>
#include <pin.h>
/*----------------------------------------------------------------------------*/
extern const struct EntityClass * const I2cBase;
/*----------------------------------------------------------------------------*/
struct I2cBaseConfig
{
  /** Mandatory: serial clock pin. */
  pin_t scl;
  /** Mandatory: serial data pin. */
  pin_t sda;
  /** Mandatory: peripheral identifier. */
  uint8_t channel;
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
enum result i2cConfigPins(struct I2cBase *, const struct I2cBaseConfig *);
uint32_t i2cGetRate(const struct I2cBase *);
void i2cSetRate(struct I2cBase *, uint32_t);

uint32_t i2cGetClock(const struct I2cBase *);
/*----------------------------------------------------------------------------*/
#endif /* PLATFORM_NXP_I2C_BASE_H_ */
