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
  const uint32_t divisor = (clock + rate - 1) / rate;
  uint32_t clockH;
  uint32_t clockL;

  if (rate > 100000)
  {
    /* Use 11/21 duty cycle for Fast Mode and Fast Mode+ */
    clockH = (divisor * 11 + 31) >> 5;
    clockL = (divisor * 21) >> 5;
  }
  else
  {
    /* Standard Mode with 1/1 duty cycle */
    clockH = clockL = divisor >> 1;
  }

  assert(clockH >= 4 && clockL >= 4);

  if (clockH > 0xFFFF)
    clockH = 0xFFFF;
  if (clockL > 0xFFFF)
    clockL = 0xFFFF;

  reg->SCLH = clockH;
  reg->SCLL = clockL;
}
