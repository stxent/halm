/*
 * i2c_common.c
 * Copyright (C) 2023 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/delay.h>
#include <halm/platform/stm32/gen_1/i2c_defs.h>
#include <halm/platform/stm32/i2c_base.h>
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

  pin = pinInit(interface->scl);
  pinOutput(pin, true);
  pinSetType(pin, PIN_OPENDRAIN);
  pinSetFunction(pin, pinEntry->value);

  /* Configure I2C serial data pin */
  pinEntry = pinFind(i2cPins, interface->sda, interface->channel);
  assert(pinEntry != NULL);

  pin = pinInit(interface->sda);
  pinOutput(pin, true);
  pinSetType(pin, PIN_OPENDRAIN);
  pinSetFunction(pin, pinEntry->value);
}
/*----------------------------------------------------------------------------*/
uint32_t i2cGetRate(const struct I2CBase *interface)
{
  const STM_I2C_Type * const reg = interface->reg;
  const uint32_t clock = i2cGetClock(interface);
  const uint32_t ccr = reg->CCR;
  const uint32_t divisor = CCR_CCR_VALUE(ccr);
  uint32_t rate;

  if (ccr & CCR_FS)
  {
    if (ccr & CCR_DUTY)
      rate = (clock / divisor) * 25;
    else
      rate = (clock / divisor) * 3;
  }
  else
    rate = (clock / divisor) * 2;

  return rate;
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
  STM_I2C_Type * const reg = interface->reg;
  const uint32_t clock = i2cGetClock(interface);
  const uint32_t enabled = reg->CR1 & CR1_PE;
  uint32_t ccr;
  uint32_t trise;

  assert(rate != 0 && clock >= 2000000);

  if (rate > 100000)
  {
    assert((clock / 25 + rate - 1) / rate <= CCR_CCR_MAX);

    /* Enable Fast Mode and 16/9 duty cycle */
    ccr = CCR_FS | CCR_DUTY | CCR_CCR((clock / 25 + rate - 1) / rate);
    /* Rise time is 300 ns for fast mode */
    trise = (3 * clock) / 10000000 + 1;

    assert(CCR_CCR_VALUE(ccr) >= 0x01);
  }
  else
  {
    assert((clock / 2 + rate - 1) / rate <= CCR_CCR_MAX);

    /* Standard Mode with 1/1 duty cycle */
    ccr = CCR_CCR((clock / 2 + rate - 1) / rate);
    /* Rise time is 1000 ns for standard mode */
    trise = clock / 1000000 + 1;

    assert(CCR_CCR_VALUE(ccr) >= 0x04);
  }

  reg->CR1 &= ~CR1_PE;
  reg->CR2 = (reg->CR2 & ~CR2_FREQ_MASK) | CR2_FREQ(clock / 1000000);
  reg->CCR = ccr;
  reg->TRISE = trise;
  reg->CR1 |= enabled;
}
