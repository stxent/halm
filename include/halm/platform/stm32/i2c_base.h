/*
 * halm/platform/stm32/i2c_base.h
 * Copyright (C) 2023 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_STM32_I2C_BASE_H_
#define HALM_PLATFORM_STM32_I2C_BASE_H_
/*----------------------------------------------------------------------------*/
#include <halm/irq.h>
#include <halm/pin.h>
#include <xcore/interface.h>
/*----------------------------------------------------------------------------*/
#undef HEADER_PATH
#define HEADER_PATH <halm/platform/PLATFORM_TYPE/PLATFORM/i2c_base.h>
#include HEADER_PATH
#undef HEADER_PATH
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

  struct
  {
    IrqNumber er;
    IrqNumber ev;
  } irq;

  /* Serial clock pin identifier */
  PinNumber scl;
  /* Serial data pin identifier */
  PinNumber sda;
  /* Unique peripheral identifier */
  uint8_t channel;
};
/*----------------------------------------------------------------------------*/
BEGIN_DECLS

/* Common functions */
void i2cConfigPins(struct I2CBase *);
uint32_t i2cGetRate(const struct I2CBase *);
void i2cRecoverBus(struct I2CBase *);
void i2cSetRate(struct I2CBase *, uint32_t);

/* Platform-specific functions */
uint32_t i2cGetClock(const struct I2CBase *);
void *i2cMakeOneShotDma(uint8_t, uint8_t, enum DmaPriority, enum DmaType);

END_DECLS
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_STM32_I2C_BASE_H_ */
