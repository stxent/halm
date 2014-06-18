/*
 * i2c_common.c
 * Copyright (C) 2013 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <platform/nxp/i2c_base.h>
#include <platform/nxp/i2c_defs.h>
/*----------------------------------------------------------------------------*/
extern const struct GpioDescriptor i2cPins[];
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
enum result i2cSetupPins(struct I2cBase *interface,
    const struct I2cBaseConfig *config)
{
  const struct GpioDescriptor *pinDescriptor;
  struct Gpio pin;

  /* Setup I2C serial clock pin */
  if (!(pinDescriptor = gpioFind(i2cPins, config->scl, interface->channel)))
    return E_VALUE;
  gpioInput((pin = gpioInit(config->scl)));
  gpioSetFunction(pin, pinDescriptor->value);

  /* Setup I2C serial data pin */
  if (!(pinDescriptor = gpioFind(i2cPins, config->sda, interface->channel)))
    return E_VALUE;
  gpioInput((pin = gpioInit(config->sda)));
  gpioSetFunction(pin, pinDescriptor->value);

  return E_OK;
}
