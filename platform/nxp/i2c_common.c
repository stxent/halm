/*
 * i2c_common.c
 * Copyright (C) 2013 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <platform/nxp/i2c_base.h>
#include <platform/nxp/i2c_defs.h>
/*----------------------------------------------------------------------------*/
extern const struct PinEntry i2cPins[];
/*----------------------------------------------------------------------------*/
uint32_t i2cGetRate(const struct I2cBase *interface)
{
  LPC_I2C_Type * const reg = interface->reg;
  const uint32_t rate = reg->SCLL + reg->SCLH;

  return rate ? i2cGetClock(interface) / rate : 0;
}
/*----------------------------------------------------------------------------*/
void i2cSetRate(struct I2cBase *interface, uint32_t rate)
{
  LPC_I2C_Type * const reg = interface->reg;

  reg->SCLL = reg->SCLH = (i2cGetClock(interface) >> 1) / rate;
}
/*----------------------------------------------------------------------------*/
enum result i2cConfigPins(struct I2cBase *interface,
    const struct I2cBaseConfig *config)
{
  const struct PinEntry *pinEntry;
  struct Pin pin;

  /* Configure I2C serial clock pin */
  if (!(pinEntry = pinFind(i2cPins, config->scl, interface->channel)))
    return E_VALUE;
  pinInput((pin = pinInit(config->scl)));
  pinSetFunction(pin, pinEntry->value);
  pinSetSlewRate(pin, PIN_SLEW_FAST);

  /* Configure I2C serial data pin */
  if (!(pinEntry = pinFind(i2cPins, config->sda, interface->channel)))
    return E_VALUE;
  pinInput((pin = pinInit(config->sda)));
  pinSetFunction(pin, pinEntry->value);
  pinSetSlewRate(pin, PIN_SLEW_FAST);

  return E_OK;
}
