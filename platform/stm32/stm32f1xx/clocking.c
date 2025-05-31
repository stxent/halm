/*
 * clocking.c
 * Copyright (C) 2016 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/clock.h>
#include <halm/platform/platform_defs.h>
#include <halm/platform/stm32/clocking.h>
#include <halm/platform/stm32/stm32f1xx/clocking_defs.h>
#include <halm/platform/stm32/system.h>
#include <xcore/accel.h>
#include <assert.h>
#include <stddef.h>
/*----------------------------------------------------------------------------*/
#define HSI_OSC_FREQUENCY 8000000
#define LSI_OSC_FREQUENCY 40000

#define TICK_RATE(frequency) \
    (((frequency) * 4398046ULL + (1ULL << 31)) >> 32)
/*----------------------------------------------------------------------------*/
static uint32_t adcPrescalerToValue(uint32_t);
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

static void mainPllDisable(const void *);
static enum Result mainPllEnable(const void *, const void *);
static uint32_t mainPllFrequency(const void *);
static bool mainPllReady(const void *);

static enum Result systemClockEnable(const void *, const void *);
static uint32_t systemClockFrequency(const void *);

static enum Result usbClockEnable(const void *, const void *);
static uint32_t usbClockFrequency(const void *);

static enum Result adcClockEnable(const void *, const void *);
static uint32_t adcClockFrequency(const void *);
static enum Result apb1ClockEnable(const void *, const void *);
static uint32_t apb1ClockFrequency(const void *);
static enum Result apb2ClockEnable(const void *, const void *);
static uint32_t apb2ClockFrequency(const void *);
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

const struct ClockClass * const MainPll = &(const struct ClockClass){
    .disable = mainPllDisable,
    .enable = mainPllEnable,
    .frequency = mainPllFrequency,
    .ready = mainPllReady
};

const struct ClockClass * const SystemClock = &(const struct ClockClass){
    .disable = clockDisableStub,
    .enable = systemClockEnable,
    .frequency = systemClockFrequency,
    .ready = clockReadyStub
};

const struct ClockClass * const UsbClock = &(const struct ClockClass){
    .disable = clockDisableStub,
    .enable = usbClockEnable,
    .frequency = usbClockFrequency,
    .ready = clockReadyStub
};

const struct ClockClass * const AdcClock = &(const struct ClockClass){
    .disable = clockDisableStub,
    .enable = adcClockEnable,
    .frequency = adcClockFrequency,
    .ready = clockReadyStub
};

const struct ClockClass * const Apb1Clock = &(const struct ClockClass){
    .disable = clockDisableStub,
    .enable = apb1ClockEnable,
    .frequency = apb1ClockFrequency,
    .ready = clockReadyStub
};

const struct ClockClass * const Apb2Clock = &(const struct ClockClass){
    .disable = clockDisableStub,
    .enable = apb2ClockEnable,
    .frequency = apb2ClockFrequency,
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
static uint32_t adcPrescalerToValue(uint32_t adcpre)
{
  return 2 * (adcpre + 1);
}
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
  sysFlashLatencyUpdate(3);
}
/*----------------------------------------------------------------------------*/
static void flashLatencyUpdate(uint32_t frequency)
{
  static const uint32_t frequencyStep = 24000000;
  const unsigned int clocks = (frequency + (frequencyStep - 1)) / frequencyStep;

  sysFlashLatencyUpdate(MIN(clocks, 3));
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
static void clockDisableStub(const void *)
{
}
/*----------------------------------------------------------------------------*/
static bool clockReadyStub(const void *)
{
  return true;
}
/*----------------------------------------------------------------------------*/
static void extOscDisable(const void *)
{
  STM_RCC->CR &= ~CR_HSEON;
}
/*----------------------------------------------------------------------------*/
static enum Result extOscEnable(const void *, const void *configBase)
{
  const struct ExternalOscConfig * const config = configBase;
  assert(config != NULL);
  assert(config->frequency >= 4000000 && config->frequency <= 16000000);

  uint32_t control = (STM_RCC->CR & ~(CR_HSERDY | CR_HSEBYP)) | CR_HSEON;

  if (config->bypass)
    control |= CR_HSEBYP;

  /* Power-up oscillator */
  STM_RCC->CR = control;

  extFrequency = config->frequency;
  return E_OK;
}
/*----------------------------------------------------------------------------*/
static uint32_t extOscFrequency(const void *)
{
  return (STM_RCC->CR & CR_HSERDY) ? extFrequency : 0;
}
/*----------------------------------------------------------------------------*/
static bool extOscReady(const void *)
{
  return extFrequency && (STM_RCC->CR & CR_HSERDY) != 0;
}
/*----------------------------------------------------------------------------*/
static void intLowSpeedOscDisable(const void *)
{
  STM_RCC->CSR &= ~CSR_LSION;
}
/*----------------------------------------------------------------------------*/
static enum Result intLowSpeedOscEnable(const void *, const void *)
{
  STM_RCC->CSR |= CSR_LSION;
  return E_OK;
}
/*----------------------------------------------------------------------------*/
static uint32_t intLowSpeedOscFrequency(const void *)
{
  return (STM_RCC->CSR & CSR_LSIRDY) != 0 ? LSI_OSC_FREQUENCY : 0;
}
/*----------------------------------------------------------------------------*/
static bool intLowSpeedOscReady(const void *)
{
  return (STM_RCC->CSR & CSR_LSIRDY) != 0;
}
/*----------------------------------------------------------------------------*/
static void intOscDisable(const void *)
{
  STM_RCC->CR &= ~CR_HSION;
}
/*----------------------------------------------------------------------------*/
static enum Result intOscEnable(const void *, const void *)
{
  STM_RCC->CR |= CR_HSION;
  return E_OK;
}
/*----------------------------------------------------------------------------*/
static uint32_t intOscFrequency(const void *)
{
  return (STM_RCC->CR & CR_HSIRDY) != 0 ? HSI_OSC_FREQUENCY : 0;
}
/*----------------------------------------------------------------------------*/
static bool intOscReady(const void *)
{
  return (STM_RCC->CR & CR_HSIRDY) != 0;
}
/*----------------------------------------------------------------------------*/
static void mainPllDisable(const void *)
{
  STM_RCC->CR &= ~CR_PLLON;
}
/*----------------------------------------------------------------------------*/
static enum Result mainPllEnable(const void *, const void *configBase)
{
  // TODO Detect chip
  const struct MainPllConfig * const config = configBase;
  assert(config != NULL);
  assert(config->multiplier >= 2 && config->multiplier <= 16);

  uint32_t frequency;

  if (config->source == CLOCK_INTERNAL)
  {
    assert(config->divisor == 2);

    STM_RCC->CFGR &= ~CFGR_PLLSRC;
    frequency = HSI_OSC_FREQUENCY / 2;
  }
  else
  {
    assert(config->divisor >= 1 && config->divisor <= 2);

    if (config->divisor == 1)
    {
      STM_RCC->CFGR &= ~CFGR_PLLXTPRE;
      frequency = extFrequency;
    }
    else
    {
      STM_RCC->CFGR |= CFGR_PLLXTPRE;
      frequency = extFrequency / 2;
    }

    STM_RCC->CFGR |= CFGR_PLLSRC;
  }

  STM_RCC->CFGR = (STM_RCC->CFGR & ~CFGR_PLLMUL_MASK)
      | CFGR_PLLMUL(config->multiplier - 2);
  STM_RCC->CR |= CR_PLLON;
  pllFrequency = frequency * config->multiplier;

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static uint32_t mainPllFrequency(const void *)
{
  return (STM_RCC->CR & CR_PLLRDY) ? pllFrequency : 0;
}
/*----------------------------------------------------------------------------*/
static bool mainPllReady(const void *)
{
  return (STM_RCC->CR & CR_PLLRDY) != 0;
}
/*----------------------------------------------------------------------------*/
static enum Result systemClockEnable(const void *, const void *configBase)
{
  const struct SystemClockConfig * const config = configBase;
  assert(config != NULL);
  assert(config->source == CLOCK_INTERNAL
      || config->source == CLOCK_EXTERNAL
      || config->source == CLOCK_PLL);

  uint32_t clockConfig = STM_RCC->CFGR & ~CFGR_SW_MASK;

  switch (config->source)
  {
    case CLOCK_INTERNAL:
      clockConfig |= CFGR_SW(CFGR_SW_HSI);
      break;

    case CLOCK_EXTERNAL:
      clockConfig |= CFGR_SW(CFGR_SW_HSE);
      break;

    case CLOCK_PLL:
      clockConfig |= CFGR_SW(CFGR_SW_PLL);
      break;

    default:
      break;
  }

  updateAhbClock(clockConfig);
  return E_OK;
}
/*----------------------------------------------------------------------------*/
static uint32_t systemClockFrequency(const void *)
{
  uint32_t frequency = 0;

  switch (CFGR_SWS_VALUE(STM_RCC->CFGR))
  {
    case CFGR_SW_HSI:
      frequency = HSI_OSC_FREQUENCY;
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
static enum Result usbClockEnable(const void *, const void *configBase)
{
  const struct UsbClockConfig * const config = configBase;
  assert(config != NULL);
  assert(config->divisor == USB_CLK_DIV_1
      || config->divisor == USB_CLK_DIV_1_5);

  if (config->divisor == USB_CLK_DIV_1_5)
    STM_RCC->CFGR &= ~CFGR_USBPRE;
  else
    STM_RCC->CFGR |= CFGR_USBPRE;

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static uint32_t usbClockFrequency(const void *)
{
  if (STM_RCC->CFGR & CFGR_USBPRE)
    return pllFrequency;
  else
    return (pllFrequency * 3) / 2;
}
/*----------------------------------------------------------------------------*/
static enum Result adcClockEnable(const void *, const void *configBase)
{
  const struct BusClockConfig * const config = configBase;
  assert(config != NULL);

  const unsigned int prescaler = config->divisor / 2 - 1;
  assert(prescaler < 4 && adcPrescalerToValue(prescaler) == config->divisor);

  STM_RCC->CFGR = (STM_RCC->CFGR & ~CFGR_ADCPRE_MASK) | CFGR_ADCPRE(prescaler);
  return E_OK;
}
/*----------------------------------------------------------------------------*/
static uint32_t adcClockFrequency(const void *)
{
  const uint32_t divisor =
      adcPrescalerToValue(CFGR_ADCPRE_VALUE(STM_RCC->CFGR));
  return apb2ClockFrequency(NULL) / divisor;
}
/*----------------------------------------------------------------------------*/
static enum Result apb1ClockEnable(const void *, const void *configBase)
{
  const struct BusClockConfig * const config = configBase;
  assert(config != NULL);
  assert(config->divisor);

  const unsigned int prescaler = 31 - countLeadingZeros32(config->divisor);
  assert(prescaler < 5 && 1 << prescaler == config->divisor);

  const unsigned int value = prescaler >= 1 ? 0x04 | (prescaler - 1) : 0;
  STM_RCC->CFGR = (STM_RCC->CFGR & ~CFGR_PPRE1_MASK) | CFGR_PPRE1(value);

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static uint32_t apb1ClockFrequency(const void *)
{
  const uint32_t divisor = apbPrescalerToValue(CFGR_PPRE1_VALUE(STM_RCC->CFGR));
  return mainClockFrequency(NULL) / divisor;
}
/*----------------------------------------------------------------------------*/
static enum Result apb2ClockEnable(const void *, const void *configBase)
{
  const struct BusClockConfig * const config = configBase;
  assert(config != NULL);
  assert(config->divisor);

  const unsigned int prescaler = 31 - countLeadingZeros32(config->divisor);
  assert(prescaler < 5 && 1 << prescaler == config->divisor);

  const unsigned int value = prescaler >= 1 ? 0x04 | (prescaler - 1) : 0;
  STM_RCC->CFGR = (STM_RCC->CFGR & ~CFGR_PPRE2_MASK) | CFGR_PPRE2(value);

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static uint32_t apb2ClockFrequency(const void *)
{
  const uint32_t divisor = apbPrescalerToValue(CFGR_PPRE2_VALUE(STM_RCC->CFGR));
  return mainClockFrequency(NULL) / divisor;
}
/*----------------------------------------------------------------------------*/
static enum Result mainClockEnable(const void *, const void *configBase)
{
  const struct BusClockConfig * const config = configBase;
  assert(config != NULL);
  assert(config->divisor);

  const unsigned int prescaler = 31 - countLeadingZeros32(config->divisor);
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
static uint32_t mainClockFrequency(const void *)
{
  const uint32_t divisor = ahbPrescalerToValue(CFGR_HPRE_VALUE(STM_RCC->CFGR));
  return systemClockFrequency(NULL) / divisor;
}
