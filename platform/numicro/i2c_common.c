/*
 * i2c_common.c
 * Copyright (C) 2023 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/delay.h>
#include <halm/platform/numicro/i2c_base.h>
#include <halm/platform/numicro/i2c_defs.h>
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
  assert(pinEntry != NULL);
  pinInput((pin = pinInit(interface->scl)));
  pinSetFunction(pin, pinEntry->value);
  pinSetSlewRate(pin, PIN_SLEW_FAST);

  /* Configure I2C serial data pin */
  pinEntry = pinFind(i2cPins, interface->sda, interface->channel);
  assert(pinEntry != NULL);
  pinInput((pin = pinInit(interface->sda)));
  pinSetFunction(pin, pinEntry->value);
  pinSetSlewRate(pin, PIN_SLEW_FAST);
}
/*----------------------------------------------------------------------------*/
uint32_t i2cGetRate(const struct I2CBase *interface)
{
  const NM_I2C_Type * const reg = interface->reg;
  const uint32_t divisor = (reg->CLKDIV + 1) * 4;

  return i2cGetClock(interface) / divisor;
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

  NM_I2C_Type * const reg = interface->reg;
  const uint32_t clock = i2cGetClock(interface) >> 2;
  uint32_t divisor = (clock + (rate - 1)) / rate;

  if (divisor < 5)
    divisor = 5;
  if (divisor > CLKDIV_MAX)
    divisor = CLKDIV_MAX;

  reg->CLKDIV = divisor;
  // TODO Calculate timings
  reg->TMCTL = TMCTL_STCTL(0) | TMCTL_HTCTL(0);
}
