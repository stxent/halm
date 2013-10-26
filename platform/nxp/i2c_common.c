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
uint32_t i2cGetRate(struct I2cBase *interface)
{
  LPC_I2C_Type *reg = interface->reg;
  uint32_t rate = reg->SCLL + reg->SCLH;

  return rate ? i2cGetClock(interface) / rate : 0;
}
/*----------------------------------------------------------------------------*/
void i2cSetRate(struct I2cBase *interface, uint32_t rate)
{
  LPC_I2C_Type *reg = interface->reg;

  reg->SCLL = reg->SCLH = (i2cGetClock(interface) >> 1) / rate;
}
/*----------------------------------------------------------------------------*/
enum result i2cSetupPins(struct I2cBase *interface,
    const struct I2cBaseConfig *config)
{
  const struct GpioDescriptor *pin;

  /* Setup I2C serial clock pin */
  if (!(pin = gpioFind(i2cPins, config->scl, interface->channel)))
    return E_VALUE;
  interface->sclPin = gpioInit(config->scl, GPIO_INPUT);
  gpioSetFunction(&interface->sclPin, pin->value);

  /* Setup I2C serial data pin */
  if (!(pin = gpioFind(i2cPins, config->sda, interface->channel)))
    return E_VALUE;
  interface->sdaPin = gpioInit(config->sda, GPIO_INPUT);
  gpioSetFunction(&interface->sdaPin, pin->value);

  return E_OK;
}
