/*
 * i2c_common.c
 * Copyright (C) 2013 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <halm/platform/nxp/gen_1/i2c_base.h>
#include <halm/platform/nxp/gen_1/i2c_defs.h>
#include <assert.h>
/*----------------------------------------------------------------------------*/
extern const struct PinEntry i2cPins[];
/*----------------------------------------------------------------------------*/
uint32_t i2cGetRate(const struct I2CBase *interface)
{
  LPC_I2C_Type * const reg = interface->reg;
  const uint32_t rate = reg->SCLL + reg->SCLH;

  return rate ? i2cGetClock(interface) / rate : 0;
}
/*----------------------------------------------------------------------------*/
void i2cSetRate(struct I2CBase *interface, uint32_t rate)
{
  assert(rate != 0);

  LPC_I2C_Type * const reg = interface->reg;
  const uint32_t clock = i2cGetClock(interface);
  uint32_t divisor = ((clock + (rate - 1)) >> 1) / rate;

  if (divisor > 0xFFFF)
    divisor = 0xFFFF;

  reg->SCLL = reg->SCLH = divisor;
}
/*----------------------------------------------------------------------------*/
void i2cConfigPins(struct I2CBase *interface,
    const struct I2CBaseConfig *config)
{
  const struct PinEntry *pinEntry;
  struct Pin pin;

  /* Configure I2C serial clock pin */
  pinEntry = pinFind(i2cPins, config->scl, interface->channel);
  assert(pinEntry);
  pinInput((pin = pinInit(config->scl)));
  pinSetFunction(pin, pinEntry->value);
  pinSetSlewRate(pin, PIN_SLEW_FAST);

  /* Configure I2C serial data pin */
  pinEntry = pinFind(i2cPins, config->sda, interface->channel);
  assert(pinEntry);
  pinInput((pin = pinInit(config->sda)));
  pinSetFunction(pin, pinEntry->value);
  pinSetSlewRate(pin, PIN_SLEW_FAST);
}
