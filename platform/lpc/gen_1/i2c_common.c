/*
 * i2c_common.c
 * Copyright (C) 2013 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/delay.h>
#include <halm/platform/lpc/gen_1/i2c_defs.h>
#include <halm/platform/lpc/i2c_base.h>
#include <assert.h>
/*----------------------------------------------------------------------------*/
#define RECOVERY_CYCLES 10
/* High and low time in microseconds during bus recover */
#define RECOVERY_TIME   20
/*----------------------------------------------------------------------------*/
extern const struct PinEntry i2cPins[];
/*----------------------------------------------------------------------------*/
void i2cConfigPins(struct I2CBase *interface)
{
  const struct PinEntry *pinEntry;
  struct Pin pin;

  /* Configure I2C serial clock pin */
  pinEntry = pinFind(i2cPins, interface->scl, interface->channel);
  assert(pinEntry);
  pinInput((pin = pinInit(interface->scl)));
  pinSetFunction(pin, pinEntry->value);
  pinSetSlewRate(pin, PIN_SLEW_FAST);

  /* Configure I2C serial data pin */
  pinEntry = pinFind(i2cPins, interface->sda, interface->channel);
  assert(pinEntry);
  pinInput((pin = pinInit(interface->sda)));
  pinSetFunction(pin, pinEntry->value);
  pinSetSlewRate(pin, PIN_SLEW_FAST);
}
/*----------------------------------------------------------------------------*/
uint32_t i2cGetRate(const struct I2CBase *interface)
{
  const LPC_I2C_Type * const reg = interface->reg;
  const uint32_t prescaler = reg->SCLL + reg->SCLH;

  return prescaler ? i2cGetClock(interface) / prescaler : 0;
}
/*----------------------------------------------------------------------------*/
void i2cRecoverBus(struct I2CBase *interface)
{
  /* Configure SCL pin as an open-drain output */
  const struct Pin scl = pinInit(interface->scl);
  pinOutput(scl, true);
  pinSetType(scl, PIN_OPENDRAIN);

  /* Configure SDA pin as an open-drain output, set logic high */
  const struct Pin sda = pinInit(interface->sda);
  pinOutput(sda, true);
  pinSetType(sda, PIN_OPENDRAIN);

  for (unsigned int number = 0; number < RECOVERY_CYCLES; ++number)
  {
    udelay(RECOVERY_TIME);
    pinReset(scl);
    udelay(RECOVERY_TIME);
    pinSet(scl);
  }
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
