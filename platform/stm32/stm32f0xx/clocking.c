/*
 * clocking.c
 * Copyright (C) 2020 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/clock.h>
#include <halm/platform/platform_defs.h>
#include <halm/platform/stm32/clocking.h>
#include <halm/platform/stm32/stm32f0xx/clocking_defs.h>
#include <halm/platform/stm32/system.h>
#include <xcore/accel.h>
#include <assert.h>
#include <stdbool.h>
/*----------------------------------------------------------------------------*/
#define HSI_OSC_FREQUENCY     8000000
#define HSI_OSC_48_FREQUENCY  48000000
#define TICK_RATE(frequency)  ((frequency) / 1000)
/*----------------------------------------------------------------------------*/
//static uint32_t adcPrescalerToValue(uint32_t);
static uint32_t ahbPrescalerToValue(uint32_t);
static uint32_t apbPrescalerToValue(uint32_t);
static inline void flashLatencyReset(void);
static void flashLatencyUpdate(uint32_t);
static void updateAhbClock(uint32_t);

static void clockDisableStub(const void *);
static bool clockReadyStub(const void *);
/*----------------------------------------------------------------------------*/
static void extOscDisable(const void *);
static enum Result extOscEnable(const void *, const void *);
static uint32_t extOscFrequency(const void *);
static bool extOscReady(const void *);

static void intOscDisable(const void *);
static enum Result intOscEnable(const void *, const void *);
static uint32_t intOscFrequency(const void *);
static bool intOscReady(const void *);

static void intOsc48Disable(const void *);
static enum Result intOsc48Enable(const void *, const void *);
static uint32_t intOsc48Frequency(const void *);
static bool intOsc48Ready(const void *);

static void systemPllDisable(const void *);
static enum Result systemPllEnable(const void *, const void *);
static uint32_t systemPllFrequency(const void *);
static bool systemPllReady(const void *);

static enum Result systemClockEnable(const void *, const void *);
static uint32_t systemClockFrequency(const void *);

static enum Result mainClockEnable(const void *, const void *);
static uint32_t mainClockFrequency(const void *);
//static enum Result adcClockEnable(const void *, const void *);
//static uint32_t adcClockFrequency(const void *);
static enum Result apbClockEnable(const void *, const void *);
static uint32_t apbClockFrequency(const void *);
/*----------------------------------------------------------------------------*/
const struct ClockClass * const ExternalOsc = &(const struct ClockClass){
    .disable = extOscDisable,
    .enable = extOscEnable,
    .frequency = extOscFrequency,
    .ready = extOscReady
};

const struct ClockClass * const InternalOsc = &(const struct ClockClass){
    .disable = intOscDisable,
    .enable = intOscEnable,
    .frequency = intOscFrequency,
    .ready = intOscReady
};

const struct ClockClass * const InternalOsc48 = &(const struct ClockClass){
    .disable = intOsc48Disable,
    .enable = intOsc48Enable,
    .frequency = intOsc48Frequency,
    .ready = intOsc48Ready
};

const struct ClockClass * const SystemClock = &(const struct ClockClass){
    .disable = clockDisableStub,
    .enable = systemClockEnable,
    .frequency = systemClockFrequency,
    .ready = clockReadyStub
};

const struct ClockClass * const SystemPll = &(const struct ClockClass){
    .disable = systemPllDisable,
    .enable = systemPllEnable,
    .frequency = systemPllFrequency,
    .ready = systemPllReady
};

const struct ClockClass * const MainClock = &(const struct ClockClass){
    .disable = clockDisableStub,
    .enable = mainClockEnable,
    .frequency = mainClockFrequency,
    .ready = clockReadyStub
};

//const struct ClockClass * const AdcClock = &(const struct ClockClass){
//    .disable = clockDisableStub,
//    .enable = adcClockEnable,
//    .frequency = adcClockFrequency,
//    .ready = clockReadyStub
//};

const struct ClockClass * const ApbClock = &(const struct ClockClass){
    .disable = clockDisableStub,
    .enable = apbClockEnable,
    .frequency = apbClockFrequency,
    .ready = clockReadyStub
};
/*----------------------------------------------------------------------------*/
static uint32_t extFrequency = 0;
static uint32_t pllFrequency = 0;
uint32_t ticksPerSecond = TICK_RATE(HSI_OSC_FREQUENCY);
/*----------------------------------------------------------------------------*/
//static uint32_t adcPrescalerToValue(uint32_t adcpre)
//{
//  return 2 * (adcpre + 1);
//}
/*----------------------------------------------------------------------------*/
static uint32_t ahbPrescalerToValue(uint32_t hpre)
{
  if (hpre >= 8)
  {
    hpre &= 0x07;
    return hpre < 4 ? (2 << hpre) : (4 << hpre);
  }
  else
    return 1;
}
/*----------------------------------------------------------------------------*/
static uint32_t apbPrescalerToValue(uint32_t ppre)
{
  return ppre < 4 ? 1 : (2 << (ppre & 0x03));
}
/*----------------------------------------------------------------------------*/
static inline void flashLatencyReset(void)
{
  /* Select safe setting */
  sysFlashLatencyUpdate(2);
}
/*----------------------------------------------------------------------------*/
static void flashLatencyUpdate(uint32_t frequency)
{
  const unsigned int clocks = frequency > 24000000 ? 2 : 1;
  sysFlashLatencyUpdate(MIN(clocks, 2));
}
/*----------------------------------------------------------------------------*/
static void updateAhbClock(uint32_t configuration)
{
  /* Set highest flash latency */
  flashLatencyReset();
  /* Update clock settings */
  STM_RCC->CFGR = configuration;

  /* Recalculate flash latency and tick rate */
  const uint32_t frequency = mainClockFrequency(0);

  flashLatencyUpdate(frequency);
  ticksPerSecond = TICK_RATE(frequency);
}
/*----------------------------------------------------------------------------*/
static void clockDisableStub(const void *clockBase __attribute__((unused)))
{
}
/*----------------------------------------------------------------------------*/
static bool clockReadyStub(const void *clockBase __attribute__((unused)))
{
  return true;
}
/*----------------------------------------------------------------------------*/
static void extOscDisable(const void *clockBase __attribute__((unused)))
{
  STM_RCC->CR &= ~CR_HSEON;
  extFrequency = 0;
}
/*----------------------------------------------------------------------------*/
static enum Result extOscEnable(const void *clockBase
    __attribute__((unused)), const void *configBase)
{
  const struct ExternalOscConfig * const config = configBase;
  assert(config->frequency >= 4000000 && config->frequency <= 32000000);

  uint32_t control = (STM_RCC->CR & ~(CR_HSERDY | CR_HSEBYP)) | CR_HSEON;

  if (config->bypass)
    control |= CR_HSEBYP;

  /* Power-up oscillator */
  STM_RCC->CR = control;

  extFrequency = config->frequency;
  return E_OK;
}
/*----------------------------------------------------------------------------*/
static uint32_t extOscFrequency(const void *clockBase __attribute__((unused)))
{
  return extFrequency;
}
/*----------------------------------------------------------------------------*/
static bool extOscReady(const void *clockBase __attribute__((unused)))
{
  return (STM_RCC->CR & CR_HSERDY) != 0;
}
/*----------------------------------------------------------------------------*/
static void intOscDisable(const void *clockBase __attribute__((unused)))
{
  STM_RCC->CR &= ~CR_HSION;
}
/*----------------------------------------------------------------------------*/
static enum Result intOscEnable(const void *clockBase __attribute__((unused)),
    const void *configBase __attribute__((unused)))
{
  STM_RCC->CR |= CR_HSION;
  return E_OK;
}
/*----------------------------------------------------------------------------*/
static uint32_t intOscFrequency(const void *clockBase __attribute__((unused)))
{
  return (STM_RCC->CR & CR_HSIRDY) != 0 ? HSI_OSC_FREQUENCY : 0;
}
/*----------------------------------------------------------------------------*/
static bool intOscReady(const void *clockBase __attribute__((unused)))
{
  return (STM_RCC->CR & CR_HSIRDY) != 0;
}
/*----------------------------------------------------------------------------*/
static void intOsc48Disable(const void *clockBase __attribute__((unused)))
{
  STM_RCC->CR2 &= ~CR2_HSI48ON;
}
/*----------------------------------------------------------------------------*/
static enum Result intOsc48Enable(const void *clockBase __attribute__((unused)),
    const void *configBase __attribute__((unused)))
{
  STM_RCC->CR2 |= CR2_HSI48ON;
  return E_OK;
}
/*----------------------------------------------------------------------------*/
static uint32_t intOsc48Frequency(const void *clockBase __attribute__((unused)))
{
  return (STM_RCC->CR2 & CR2_HSI48RDY) != 0 ? HSI_OSC_48_FREQUENCY : 0;
}
/*----------------------------------------------------------------------------*/
static bool intOsc48Ready(const void *clockBase __attribute__((unused)))
{
  return (STM_RCC->CR2 & CR2_HSI48RDY) != 0;
}
/*----------------------------------------------------------------------------*/
static void systemPllDisable(const void *clockBase __attribute__((unused)))
{
  STM_RCC->CR &= ~CR_PLLON;
}
/*----------------------------------------------------------------------------*/
static enum Result systemPllEnable(const void *clockBase
    __attribute__((unused)), const void *configBase)
{
  const struct SystemPllConfig * const config = configBase;
  assert(config);
  assert(config->source == CLOCK_INTERNAL
      || config->source == CLOCK_INTERNAL_48
      || config->source == CLOCK_EXTERNAL);
  assert(config->multiplier >= 2 && config->multiplier <= 16);
  assert(config->divisor >= 1 && config->divisor <= 16);

  uint32_t cfgr = STM_RCC->CFGR & ~(CFGR_PLLSRC_MASK | CFGR_PLLMUL_MASK);
  uint32_t cfgr2 = STM_RCC->CFGR2 & ~CFGR2_PREDIV_MASK;
  uint32_t frequency;

  switch (config->source)
  {
    case CLOCK_INTERNAL:
    {
      cfgr |= CFGR_PLLSRC(CFGR_PLLSRC_HSI);
      frequency = HSI_OSC_FREQUENCY;
      break;
    }

    case CLOCK_INTERNAL_48:
    {
      cfgr |= CFGR_PLLSRC(CFGR_PLLSRC_HSI48);
      frequency = HSI_OSC_48_FREQUENCY;
      break;
    }

    case CLOCK_EXTERNAL:
    {
      cfgr |= CFGR_PLLSRC(CFGR_PLLSRC_HSE);
      frequency = extFrequency;
      break;
    }

    default:
      return E_VALUE;
  }

  cfgr |= CFGR_PLLMUL(config->multiplier - 2);
  cfgr2 |= CFGR2_PREDIV(config->divisor - 1);

  STM_RCC->CFGR = cfgr;
  STM_RCC->CFGR2 = cfgr2;
  STM_RCC->CR |= CR_PLLON;

  pllFrequency = (frequency / config->divisor) * config->multiplier;
  return E_OK;
}
/*----------------------------------------------------------------------------*/
static uint32_t systemPllFrequency(const void *clockBase
    __attribute__((unused)))
{
  return pllFrequency;
}
/*----------------------------------------------------------------------------*/
static bool systemPllReady(const void *clockBase __attribute__((unused)))
{
  return (STM_RCC->CR & CR_PLLRDY) != 0;
}
/*----------------------------------------------------------------------------*/
static enum Result systemClockEnable(const void *clockBase
    __attribute__((unused)), const void *configBase)
{
  const struct SystemClockConfig * const config = configBase;
  assert(config);
  assert(config->source == CLOCK_INTERNAL
      || config->source == CLOCK_INTERNAL_48
      || config->source == CLOCK_EXTERNAL
      || config->source == CLOCK_PLL);

  uint32_t cfgr = STM_RCC->CFGR & ~CFGR_SW_MASK;

  switch (config->source)
  {
    case CLOCK_INTERNAL:
      cfgr |= CFGR_SW(CFGR_SW_HSI);
      break;

    case CLOCK_INTERNAL_48:
      cfgr |= CFGR_SW(CFGR_SW_HSI48);
      break;

    case CLOCK_EXTERNAL:
      cfgr |= CFGR_SW(CFGR_SW_HSE);
      break;

    case CLOCK_PLL:
      cfgr |= CFGR_SW(CFGR_SW_PLL);
      break;

    default:
      break;
  }

  updateAhbClock(cfgr);
  return E_OK;
}
/*----------------------------------------------------------------------------*/
static uint32_t systemClockFrequency(const void *clockBase
    __attribute__((unused)))
{
  uint32_t frequency = 0;

  switch (CFGR_SWS_VALUE(STM_RCC->CFGR))
  {
    case CFGR_SW_HSI:
      frequency = HSI_OSC_FREQUENCY;
      break;

    case CFGR_SW_HSI48:
      frequency = HSI_OSC_48_FREQUENCY;
      break;

    case CFGR_SW_HSE:
      frequency = extFrequency;
      break;

    case CFGR_SW_PLL:
      frequency = pllFrequency;
      break;
  }

  return frequency;
}
/*----------------------------------------------------------------------------*/
static enum Result mainClockEnable(const void *clockBase
    __attribute__((unused)), const void *configBase)
{
  const struct BusClockConfig * const config = configBase;
  assert(config);
  assert(config->divisor);

  // XXX
//  const unsigned int prescaler = 31 - countLeadingZeros32(config->divisor);
//  assert(prescaler < 10 && 1 << prescaler == config->divisor);
//
//  const unsigned int value = prescaler >= 1 ? 0x08 | (prescaler - 1) : 0;
//  updateAhbClock((STM_RCC->CFGR & ~CFGR_HPRE_MASK) | CFGR_HPRE(value));

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static uint32_t mainClockFrequency(const void *clockBase
    __attribute__((unused)))
{
  const uint32_t divisor = ahbPrescalerToValue(CFGR_HPRE_VALUE(STM_RCC->CFGR));
  return systemClockFrequency(0) / divisor;
}
/*----------------------------------------------------------------------------*/
//static enum Result adcClockEnable(const void *clockBase
//    __attribute__((unused)), const void *configBase)
//{
//  const struct BusClockConfig * const config = configBase;
//  const unsigned int prescaler = config->divisor / 2 - 1;
//  assert(prescaler < 4 && adcPrescalerToValue(prescaler) == config->divisor);
//
//  STM_RCC->CFGR = (STM_RCC->CFGR & ~CFGR_ADCPRE_MASK) | CFGR_ADCPRE(prescaler);
//  return E_OK;
//}
///*----------------------------------------------------------------------------*/
//static uint32_t adcClockFrequency(const void *clockBase
//    __attribute__((unused)))
//{
//  const uint32_t divisor =
//      adcPrescalerToValue(CFGR_ADCPRE_VALUE(STM_RCC->CFGR));
//  return apb2ClockFrequency(0) / divisor;
//}
/*----------------------------------------------------------------------------*/
static enum Result apbClockEnable(const void *clockBase
   __attribute__((unused)), const void *configBase)
{
  const struct BusClockConfig * const config = configBase;
  assert(config);
  assert(config->divisor);

  const unsigned int prescaler = 31 - countLeadingZeros32(config->divisor);
  assert(prescaler < 5 && 1 << prescaler == config->divisor);

  const unsigned int value = prescaler >= 1 ? 0x04 | (prescaler - 1) : 0;
  STM_RCC->CFGR = (STM_RCC->CFGR & ~CFGR_PPRE_MASK) | CFGR_PPRE(value);

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static uint32_t apbClockFrequency(const void *clockBase
   __attribute__((unused)))
{
  const uint32_t divisor = apbPrescalerToValue(CFGR_PPRE_VALUE(STM_RCC->CFGR));
  return mainClockFrequency(0) / divisor;
}
