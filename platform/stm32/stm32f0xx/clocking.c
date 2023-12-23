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
#include <stddef.h>
/*----------------------------------------------------------------------------*/
#define HSI_OSC_FREQUENCY     8000000
#define HSI_OSC_14_FREQUENCY  14000000
#define HSI_OSC_48_FREQUENCY  48000000
#define LSI_OSC_FREQUENCY     40000
#define TICK_RATE(frequency)  ((frequency) / 1000)
/*----------------------------------------------------------------------------*/
struct UsartClockClass
{
  struct ClockClass base;
  uint8_t offset;
};
/*----------------------------------------------------------------------------*/
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

static void intLowSpeedOscDisable(const void *);
static enum Result intLowSpeedOscEnable(const void *, const void *);
static uint32_t intLowSpeedOscFrequency(const void *);
static bool intLowSpeedOscReady(const void *);

static void intOscDisable(const void *);
static enum Result intOscEnable(const void *, const void *);
static uint32_t intOscFrequency(const void *);
static bool intOscReady(const void *);

static void intOsc14Disable(const void *);
static enum Result intOsc14Enable(const void *, const void *);
static uint32_t intOsc14Frequency(const void *);
static bool intOsc14Ready(const void *);

static void intOsc48Disable(const void *);
static enum Result intOsc48Enable(const void *, const void *);
static uint32_t intOsc48Frequency(const void *);
static bool intOsc48Ready(const void *);

static void systemPllDisable(const void *);
static enum Result systemPllEnable(const void *, const void *);
static uint32_t systemPllFrequency(const void *);
static bool systemPllReady(const void *);

static enum Result i2cClockEnable(const void *, const void *);
static uint32_t i2cClockFrequency(const void *);

static enum Result systemClockEnable(const void *, const void *);
static uint32_t systemClockFrequency(const void *);

static enum Result usartClockEnable(const void *, const void *);
static uint32_t usartClockFrequency(const void *);

static enum Result adcClockEnable(const void *, const void *);
static uint32_t adcClockFrequency(const void *);
static enum Result apbClockEnable(const void *, const void *);
static uint32_t apbClockFrequency(const void *);
static enum Result mainClockEnable(const void *, const void *);
static uint32_t mainClockFrequency(const void *);
/*----------------------------------------------------------------------------*/
const struct ClockClass * const ExternalOsc = &(const struct ClockClass){
    .disable = extOscDisable,
    .enable = extOscEnable,
    .frequency = extOscFrequency,
    .ready = extOscReady
};

const struct ClockClass * const InternalLowSpeedOsc =
    &(const struct ClockClass){
    .disable = intLowSpeedOscDisable,
    .enable = intLowSpeedOscEnable,
    .frequency = intLowSpeedOscFrequency,
    .ready = intLowSpeedOscReady
};

const struct ClockClass * const InternalOsc = &(const struct ClockClass){
    .disable = intOscDisable,
    .enable = intOscEnable,
    .frequency = intOscFrequency,
    .ready = intOscReady
};

const struct ClockClass * const InternalOsc14 = &(const struct ClockClass){
    .disable = intOsc14Disable,
    .enable = intOsc14Enable,
    .frequency = intOsc14Frequency,
    .ready = intOsc14Ready
};

const struct ClockClass * const InternalOsc48 = &(const struct ClockClass){
    .disable = intOsc48Disable,
    .enable = intOsc48Enable,
    .frequency = intOsc48Frequency,
    .ready = intOsc48Ready
};

const struct ClockClass * const I2C1Clock = &(const struct ClockClass){
    .disable = clockDisableStub,
    .enable = i2cClockEnable,
    .frequency = i2cClockFrequency,
    .ready = clockReadyStub
};

const struct ClockClass * const SystemClock = &(const struct ClockClass){
    .disable = clockDisableStub,
    .enable = systemClockEnable,
    .frequency = systemClockFrequency,
    .ready = clockReadyStub
};

const struct ClockClass * const Usart1Clock =
    (const struct ClockClass *)&(const struct UsartClockClass){
    .base = {
        .disable = clockDisableStub,
        .enable = usartClockEnable,
        .frequency = usartClockFrequency,
        .ready = clockReadyStub
    },
    .offset = 0
};

const struct ClockClass * const Usart2Clock =
    (const struct ClockClass *)&(const struct UsartClockClass){
    .base = {
        .disable = clockDisableStub,
        .enable = usartClockEnable,
        .frequency = usartClockFrequency,
        .ready = clockReadyStub
    },
    .offset = 16
};

const struct ClockClass * const Usart3Clock =
    (const struct ClockClass *)&(const struct UsartClockClass){
    .base = {
        .disable = clockDisableStub,
        .enable = usartClockEnable,
        .frequency = usartClockFrequency,
        .ready = clockReadyStub
    },
    .offset = 18
};

const struct ClockClass * const SystemPll = &(const struct ClockClass){
    .disable = systemPllDisable,
    .enable = systemPllEnable,
    .frequency = systemPllFrequency,
    .ready = systemPllReady
};

const struct ClockClass * const AdcClock = &(const struct ClockClass){
    .disable = clockDisableStub,
    .enable = adcClockEnable,
    .frequency = adcClockFrequency,
    .ready = clockReadyStub
};

const struct ClockClass * const ApbClock = &(const struct ClockClass){
    .disable = clockDisableStub,
    .enable = apbClockEnable,
    .frequency = apbClockFrequency,
    .ready = clockReadyStub
};

const struct ClockClass * const MainClock = &(const struct ClockClass){
    .disable = clockDisableStub,
    .enable = mainClockEnable,
    .frequency = mainClockFrequency,
    .ready = clockReadyStub
};
/*----------------------------------------------------------------------------*/
static uint32_t extFrequency = 0;
static uint32_t pllFrequency = 0;
uint32_t ticksPerSecond = TICK_RATE(HSI_OSC_FREQUENCY);
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
  const uint32_t frequency = mainClockFrequency(NULL);

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
  return (STM_RCC->CR & CR_HSERDY) ? extFrequency : 0;
}
/*----------------------------------------------------------------------------*/
static bool extOscReady(const void *clockBase __attribute__((unused)))
{
  return extFrequency && (STM_RCC->CR & CR_HSERDY) != 0;
}
/*----------------------------------------------------------------------------*/
static void intLowSpeedOscDisable(const void *clockBase __attribute__((unused)))
{
  STM_RCC->CSR &= ~CSR_LSION;
}
/*----------------------------------------------------------------------------*/
static enum Result intLowSpeedOscEnable(const void *clockBase
    __attribute__((unused)), const void *configBase __attribute__((unused)))
{
  STM_RCC->CSR |= CSR_LSION;
  return E_OK;
}
/*----------------------------------------------------------------------------*/
static uint32_t intLowSpeedOscFrequency(const void *clockBase
    __attribute__((unused)))
{
  return (STM_RCC->CSR & CSR_LSIRDY) != 0 ? LSI_OSC_FREQUENCY : 0;
}
/*----------------------------------------------------------------------------*/
static bool intLowSpeedOscReady(const void *clockBase __attribute__((unused)))
{
  return (STM_RCC->CSR & CSR_LSIRDY) != 0;
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
static void intOsc14Disable(const void *clockBase __attribute__((unused)))
{
  STM_RCC->CR2 &= ~CR2_HSI14ON;
}
/*----------------------------------------------------------------------------*/
static enum Result intOsc14Enable(const void *clockBase __attribute__((unused)),
    const void *configBase __attribute__((unused)))
{
  STM_RCC->CR2 = (STM_RCC->CR2 & ~CR2_HSI14DIS) | CR2_HSI14ON;
  return E_OK;
}
/*----------------------------------------------------------------------------*/
static uint32_t intOsc14Frequency(const void *clockBase __attribute__((unused)))
{
  return (STM_RCC->CR2 & CR2_HSI14RDY) != 0 ? HSI_OSC_14_FREQUENCY : 0;
}
/*----------------------------------------------------------------------------*/
static bool intOsc14Ready(const void *clockBase __attribute__((unused)))
{
  return (STM_RCC->CR2 & CR2_HSI14RDY) != 0;
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
  assert(config != NULL);
  assert(config->divisor >= 1 && config->divisor <= 16);
  assert(config->multiplier >= 2 && config->multiplier <= 16);
  assert(config->source == CLOCK_INTERNAL
      || config->source == CLOCK_INTERNAL_48
      || config->source == CLOCK_EXTERNAL);

  uint32_t cfgr = STM_RCC->CFGR & ~(CFGR_PLLSRC_MASK | CFGR_PLLMUL_MASK);
  uint32_t cfgr2 = STM_RCC->CFGR2 & ~CFGR2_PREDIV_MASK;
  uint32_t frequency;

  switch (config->source)
  {
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
    {
      cfgr |= CFGR_PLLSRC(CFGR_PLLSRC_HSI);
      frequency = HSI_OSC_FREQUENCY;
      break;
    }
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
  return (STM_RCC->CR & CR_PLLRDY) ? pllFrequency : 0;
}
/*----------------------------------------------------------------------------*/
static bool systemPllReady(const void *clockBase __attribute__((unused)))
{
  return (STM_RCC->CR & CR_PLLRDY) != 0;
}
/*----------------------------------------------------------------------------*/
static enum Result i2cClockEnable(const void *clockBase
    __attribute__((unused)), const void *configBase)
{
  const struct GenericClockConfig * const config = configBase;
  assert(config != NULL);
  assert(config->source == CLOCK_INTERNAL || config->source == CLOCK_SYSTEM);

  if (config->source == CLOCK_INTERNAL)
    STM_RCC->CFGR3 &= ~CFGR3_I2C1SW;
  else
    STM_RCC->CFGR3 |= CFGR3_I2C1SW;

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static uint32_t i2cClockFrequency(const void *clockBase
    __attribute__((unused)))
{
  const void * const clock = (STM_RCC->CFGR3 & CFGR3_I2C1SW) ?
      SystemClock : InternalOsc;

  return clockFrequency(clock);
}
/*----------------------------------------------------------------------------*/
static enum Result systemClockEnable(const void *clockBase
    __attribute__((unused)), const void *configBase)
{
  const struct GenericClockConfig * const config = configBase;
  assert(config != NULL);
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
static enum Result usartClockEnable(const void *clockBase,
    const void *configBase)
{
  const struct GenericClockConfig * const config = configBase;
  assert(config != NULL);
  assert(config->source == CLOCK_INTERNAL
      || config->source == CLOCK_RTC
      || config->source == CLOCK_APB
      || config->source == CLOCK_SYSTEM);

  const struct UsartClockClass * const clock = clockBase;
  uint32_t cfgr3 = STM_RCC->CFGR3;
  uint32_t mux;

  switch (config->source)
  {
    case CLOCK_INTERNAL:
      mux = USARTSW_HSI;
      break;

    case CLOCK_RTC:
      mux = USARTSW_LSE;
      break;

    case CLOCK_SYSTEM:
      mux = USARTSW_SYSCLK;
      break;

    default:
      mux = USARTSW_PCLK;
      break;
  }

  cfgr3 &= ~CFGR3_USARTSW_MASK(clock->offset);
  cfgr3 |= CFGR3_USARTSW(mux, clock->offset);

  STM_RCC->CFGR3 = cfgr3;
  return E_OK;
}
/*----------------------------------------------------------------------------*/
static uint32_t usartClockFrequency(const void *clockBase)
{
  const struct UsartClockClass * const clock = clockBase;
  const uint32_t mux = CFGR3_USARTSW_VALUE(STM_RCC->CFGR3, clock->offset);

  switch (mux)
  {
    case USARTSW_PCLK:
      return apbClockFrequency(NULL);

    case USARTSW_LSE:
      return 0; // TODO RtcOsc

    case USARTSW_HSI:
      return intOscFrequency(NULL);

    default:
      return systemClockFrequency(NULL);
  }
}
/*----------------------------------------------------------------------------*/
static enum Result adcClockEnable(const void *clockBase
    __attribute__((unused)), const void *configBase)
{
  const struct AdcClockConfig * const config = configBase;
  assert(config != NULL);
  assert(config->source == ADC_CLOCK_INTERNAL_14
      || config->source == ADC_CLOCK_APB_DIV_2
      || config->source == ADC_CLOCK_APB_DIV_4);

  if (!sysClockStatus(CLK_ADC))
  {
    sysClockEnable(CLK_ADC);
    sysResetEnable(RST_ADC);
    sysResetDisable(RST_ADC);
  }

  uint32_t cfgr2 = STM_ADC->CFGR2 & ~ADC_CFGR2_CKMODE_MASK;

  switch (config->source)
  {
    case ADC_CLOCK_APB_DIV_2:
      cfgr2 |= ADC_CFGR2_CKMODE(CKMODE_PCLK_DIV2);
      break;

    case ADC_CLOCK_APB_DIV_4:
      cfgr2 |= ADC_CFGR2_CKMODE(CKMODE_PCLK_DIV4);
      break;

    default:
      if (!(STM_RCC->CR2 & CR2_HSI14RDY))
        return E_ERROR;

      cfgr2 |= ADC_CFGR2_CKMODE(CKMODE_ADCCLK);
      break;
  }

  STM_ADC->CFGR2 = cfgr2;
  return E_OK;
}
/*----------------------------------------------------------------------------*/
static uint32_t adcClockFrequency(const void *clockBase
   __attribute__((unused)))
{
  const uint32_t source = ADC_CFGR2_CKMODE_VALUE(STM_ADC->CFGR2);

  switch (source)
  {
    case CKMODE_PCLK_DIV2:
      return apbClockFrequency(NULL) / 2;

    case CKMODE_PCLK_DIV4:
      return apbClockFrequency(NULL) / 4;

    default:
      return intOsc14Frequency(NULL);
  }
}
/*----------------------------------------------------------------------------*/
static enum Result apbClockEnable(const void *clockBase
   __attribute__((unused)), const void *configBase)
{
  const struct BusClockConfig * const config = configBase;
  assert(config != NULL);
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
  return mainClockFrequency(NULL) / divisor;
}
/*----------------------------------------------------------------------------*/
static enum Result mainClockEnable(const void *clockBase
    __attribute__((unused)), const void *configBase)
{
  const struct BusClockConfig * const config = configBase;
  assert(config != NULL);
  assert(config->divisor);

  unsigned int prescaler = 0;

  while ((1 << prescaler) < config->divisor)
    ++prescaler;

  assert((1 << prescaler) == config->divisor);
  assert(prescaler != 5 && prescaler <= 9);

  unsigned int value = 0;

  if (prescaler > 5)
    value = 0x08 | (prescaler - 2);
  else if (prescaler > 0)
    value = 0x08 | (prescaler - 1);

  updateAhbClock((STM_RCC->CFGR & ~CFGR_HPRE_MASK) | CFGR_HPRE(value));
  return E_OK;
}
/*----------------------------------------------------------------------------*/
static uint32_t mainClockFrequency(const void *clockBase
    __attribute__((unused)))
{
  const uint32_t divisor = ahbPrescalerToValue(CFGR_HPRE_VALUE(STM_RCC->CFGR));
  return systemClockFrequency(NULL) / divisor;
}
