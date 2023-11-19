/*
 * i2c_common.c
 * Copyright (C) 2023 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/delay.h>
#include <halm/platform/stm32/gen_2/i2c_defs.h>
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
  const uint32_t timingr = reg->TIMINGR;
  const uint32_t cycle = (TIMINGR_SCLL_VALUE(timingr) + 1)
      + (TIMINGR_SCLH_VALUE(timingr) + 1);
  const uint32_t prescaler = TIMINGR_PRESC_VALUE(timingr) + 1;

  return clock / (prescaler * cycle);
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
  static const uint32_t PERIOD_MUL = ((1ULL << 32) - 1) / (1000000000 >> 4);

  STM_I2C_Type * const reg = interface->reg;
  /* APB clock frequency in Hz must not exceed 100 MHz */
  const uint32_t frequency = i2cGetClock(interface);
  /*
   * APB clock period in ns.
   *
   * Initial formula:
   *   period = 1e9 / frequency
   * Upscale 1e9 to 2^32 - 1 and avoid usage of 64-bit division:
   *   multiplier = (2^32 - 1) / 1e9
   *   period = (2^32 - 1) / (frequency * multiplier)
   * Increase precision by additional frequency scaling:
   *   multiplier = (2^32 - 1) / (1e9 / 16)
   *   period = (2^32 - 1) / ((frequency / 16) * multiplier)
   */
  const uint32_t period = ((1ULL << 32) - 1) / ((frequency >> 4) * PERIOD_MUL);
  const uint32_t enabled = reg->CR1 & CR1_PE;
  uint32_t clockHigh;
  uint32_t clockLow;
  uint32_t dataHold;
  uint32_t dataSetup;
  uint32_t prescaler;

  assert(rate != 0 && frequency <= 100000000);

  if (rate > 400000)
  {
    /* For Fast Mode+ duty cycle 0.52 is used */
    static const uint32_t DATA_SETUP_TIME = 50;

    clockLow = (((frequency + rate - 1) / rate) * 21) >> 5;
    dataHold = 0;
    dataSetup = (DATA_SETUP_TIME + period - 1) / period;

    /* SCLL and SCLDEL fields are encoded as (value + 1) */
    if (clockLow / TIMINGR_SCLL_MAX > (dataSetup / TIMINGR_SCLDEL_MAX))
      prescaler = (clockLow + TIMINGR_SCLL_MAX - 1) / TIMINGR_SCLL_MAX;
    else
      prescaler = (dataSetup + TIMINGR_SCLDEL_MAX - 1) / TIMINGR_SCLDEL_MAX;
    assert(prescaler > 0 && prescaler <= TIMINGR_PRESC_MAX);

    const uint32_t scaledClock = frequency / prescaler;
    const uint32_t scaledPeriod = period * prescaler;

    if (prescaler != 1)
    {
      clockLow = (((scaledClock + rate - 1) / rate) * 21) >> 5;
    }

    clockHigh = (((scaledClock + rate - 1) / rate) * 11 + 31) >> 5;
    dataSetup = (DATA_SETUP_TIME + scaledPeriod - 1) / scaledPeriod;
  }
  else if (rate > 100000)
  {
    /* For Fast Mode duty cycle 0.52 is used */
    static const uint32_t DATA_HOLD_TIME = 300;
    static const uint32_t DATA_SETUP_TIME = 100;

    clockLow = (((frequency + rate - 1) / rate) * 21) >> 5;
    dataHold = (DATA_HOLD_TIME + period - 1) / period;

    /* SCLL field is encoded as (value + 1) and SDADEL is encoded as (value) */
    if (clockLow / TIMINGR_SCLL_MAX > (dataHold / TIMINGR_SDADEL_MAX))
      prescaler = (clockLow + TIMINGR_SCLL_MAX - 1) / TIMINGR_SCLL_MAX;
    else
      prescaler = (dataHold + TIMINGR_SDADEL_MAX - 1) / TIMINGR_SDADEL_MAX;
    assert(prescaler > 0 && prescaler <= TIMINGR_PRESC_MAX);

    const uint32_t scaledClock = frequency / prescaler;
    const uint32_t scaledPeriod = period * prescaler;

    if (prescaler != 1)
    {
      clockLow = (((scaledClock + rate - 1) / rate) * 21) >> 5;
      dataHold = (DATA_HOLD_TIME + scaledPeriod - 1) / scaledPeriod;
    }

    clockHigh = (((scaledClock + rate - 1) / rate) * 11 + 31) >> 5;
    dataSetup = (DATA_SETUP_TIME + scaledPeriod - 1) / scaledPeriod;
  }
  else
  {
    /* Standard Mode */
    static const uint32_t DATA_HOLD_TIME = 300;
    static const uint32_t DATA_SETUP_TIME = 250;

    clockLow = ((frequency + rate - 1) / rate) >> 1;
    dataHold = (DATA_HOLD_TIME + period - 1) / period;

    /* SCLL field is encoded as (value + 1) and SDADEL is encoded as (value) */
    if (clockLow / TIMINGR_SCLL_MAX > (dataHold / TIMINGR_SDADEL_MAX))
      prescaler = (clockLow + TIMINGR_SCLL_MAX - 1) / TIMINGR_SCLL_MAX;
    else
      prescaler = (dataHold + TIMINGR_SDADEL_MAX - 1) / TIMINGR_SDADEL_MAX;
    assert(prescaler > 0 && prescaler <= TIMINGR_PRESC_MAX);

    const uint32_t scaledClock = frequency / prescaler;
    const uint32_t scaledPeriod = period * prescaler;

    if (prescaler != 1)
    {
      clockLow = ((scaledClock + rate - 1) / rate) >> 1;
      dataHold = (DATA_HOLD_TIME + scaledPeriod - 1) / scaledPeriod;
    }

    clockHigh = clockLow;
    dataSetup = (DATA_SETUP_TIME + scaledPeriod - 1) / scaledPeriod;
  }

  reg->CR1 &= ~CR1_PE;
  reg->TIMINGR = TIMINGR_PRESC(prescaler - 1)
      | TIMINGR_SCLL(clockLow) | TIMINGR_SCLH(clockHigh)
      | TIMINGR_SDADEL(dataHold) | TIMINGR_SCLDEL(dataSetup);
  reg->CR1 |= enabled;
}
