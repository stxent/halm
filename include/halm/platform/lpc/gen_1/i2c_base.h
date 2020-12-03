/*
 * halm/platform/lpc/gen_1/i2c_base.h
 * Copyright (C) 2013 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef HALM_PLATFORM_LPC_GEN_1_I2C_BASE_H_
#define HALM_PLATFORM_LPC_GEN_1_I2C_BASE_H_
/*----------------------------------------------------------------------------*/
#include <halm/irq.h>
#include <halm/pin.h>
#include <xcore/interface.h>
/*----------------------------------------------------------------------------*/
extern const struct EntityClass * const I2CBase;

struct I2CBaseConfig
{
  /** Mandatory: serial clock pin. */
  PinNumber scl;
  /** Mandatory: serial data pin. */
  PinNumber sda;
  /** Mandatory: peripheral identifier. */
  uint8_t channel;
};

struct I2CBase
{
  struct Interface base;

  void *reg;
  void (*handler)(void *);
  IrqNumber irq;

  /* Unique peripheral identifier */
  uint8_t channel;
};
/*----------------------------------------------------------------------------*/
BEGIN_DECLS

/* Common functions */
void i2cConfigPins(struct I2CBase *, const struct I2CBaseConfig *);
uint32_t i2cGetRate(const struct I2CBase *);
void i2cSetRate(struct I2CBase *, uint32_t);

/* Platform-specific functions */
uint32_t i2cGetClock(const struct I2CBase *);

END_DECLS
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_LPC_GEN_1_I2C_BASE_H_ */
