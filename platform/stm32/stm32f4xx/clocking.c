/*
 * clocking.c
 * Copyright (C) 2024 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/clock.h>
#include <halm/platform/platform_defs.h>
#include <halm/platform/stm32/clocking.h>
#include <halm/platform/stm32/stm32f4xx/clocking_defs.h>
#include <halm/platform/stm32/system.h>
#include <xcore/accel.h>
#include <assert.h>
/*----------------------------------------------------------------------------*/
#define HSI_OSC_FREQUENCY     16000000
#define LSI_OSC_FREQUENCY     32000
#define TICK_RATE(frequency)  ((frequency) / 1000)
/*----------------------------------------------------------------------------*/
static uint32_t ahbPrescalerToValue(uint32_t);
static uint32_t apbPrescalerToValue(uint32_t);
static inline void flashLatencyReset(void);
static void flashLatencyUpdate(uint32_t, enum VoltageRange);
static void updateAhbClock(uint32_t, enum VoltageRange);

static void clockDisableStub(const void *);
static enum Result clockEnableStub(const void *, const void *);
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

static uint32_t usbClockFrequency(const void *);

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
    .enable = clockEnableStub,
    .frequency = usbClockFrequency,
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
enum VoltageRange volRange = VR_DEFAULT;
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
  sysFlashLatencyUpdate(8);
  sysPowerScalingUpdate(true);
}
/*----------------------------------------------------------------------------*/
static void flashLatencyUpdate(uint32_t frequency, enum VoltageRange range)
{
  uint32_t step;

  switch (range)
  {
    case VR_2V1_2V4:
      /* Max frequency is 138 MHz */
      step = 18000000;
      break;

    case VR_2V4_2V7:
      /* Max frequency is 168 MHz */
      step = 24000000;
      break;

    case VR_2V7_3V6:
      /* Max frequency is 168 MHz */
      step = 30000000;
      break;

    default:
      /* Safe mode, max frequency is 128 MHz */
      step = 16000000;
      break;
  }

  const unsigned int clocks = (frequency + (step - 1)) / step;

  sysFlashLatencyUpdate(MIN(clocks, 8));
  sysPowerScalingUpdate(frequency >= 144000000);
}
/*----------------------------------------------------------------------------*/
static void updateAhbClock(uint32_t configuration, enum VoltageRange range)
{
  /* Set highest flash latency */
  flashLatencyReset();
  /* Update clock settings */
  STM_RCC->CFGR = configuration;

  /* Recalculate flash latency and tick rate */
  const uint32_t frequency = mainClockFrequency(NULL);

  flashLatencyUpdate(frequency, range);
  ticksPerSecond = TICK_RATE(frequency);
}
/*----------------------------------------------------------------------------*/
static void clockDisableStub(const void *)
{
}
/*----------------------------------------------------------------------------*/
static enum Result clockEnableStub(const void *, const void *)
{
  return E_OK;
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
  assert((!config->bypass
          && config->frequency >= 4000000 && config->frequency <= 26000000)
      || (config->bypass
          && config->frequency >= 1000000 && config->frequency <= 50000000));

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
  const struct MainPllConfig * const config = configBase;
  assert(config != NULL);

  uint32_t cfgr = STM_RCC->PLLCFGR;
  uint32_t srcFrequency;
  uint32_t vcoFrequency;

  if (config->source == CLOCK_INTERNAL)
  {
    cfgr &= ~PLLCFGR_PLLSRC;
    srcFrequency = HSI_OSC_FREQUENCY;
  }
  else
  {
    cfgr |= PLLCFGR_PLLSRC;
    srcFrequency = extFrequency;
  }
  vcoFrequency = srcFrequency * config->multiplier;

  const uint32_t pllp = (config->divisor - 2) >> 1;
  if (((pllp << 1) + 2) != config->divisor || pllp > 3)
    return E_VALUE;

  const uint32_t pllm = (srcFrequency + 2000000 - 1) / 2000000;
  if (pllm < PLLCFGR_PLLM_MIN || pllm > PLLCFGR_PLLM_MAX)
    return E_VALUE;

  const uint32_t plln = vcoFrequency / (srcFrequency / pllm);
  if (plln < PLLCFGR_PLLN_MIN || plln > PLLCFGR_PLLN_MAX)
    return E_VALUE;

  const uint32_t pllq = vcoFrequency / 48000000;
  if (pllq < PLLCFGR_PLLQ_MIN || pllq > PLLCFGR_PLLQ_MAX)
    return E_VALUE;

  cfgr &= ~(PLLCFGR_PLLM_MASK | PLLCFGR_PLLN_MASK
      | PLLCFGR_PLLP_MASK | PLLCFGR_PLLQ_MASK);
  cfgr |= PLLCFGR_PLLM(pllm) | PLLCFGR_PLLN(plln)
      | PLLCFGR_PLLP(pllp) | PLLCFGR_PLLQ(pllq);

  STM_RCC->CR &= ~CR_PLLON;
  STM_RCC->PLLCFGR = cfgr;
  STM_RCC->CR |= CR_PLLON;
  pllFrequency = vcoFrequency / config->divisor;

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

  uint32_t cfgr = STM_RCC->CFGR & ~CFGR_SW_MASK;

  switch (config->source)
  {
    case CLOCK_INTERNAL:
      cfgr |= CFGR_SW(CFGR_SW_HSI);
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

  updateAhbClock(cfgr, volRange);
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
static uint32_t usbClockFrequency(const void *)
{
  if (STM_RCC->CR & CR_PLLRDY)
  {
    const uint32_t pllcfgr = STM_RCC->PLLCFGR;
    const uint32_t pllp = PLLCFGR_PLLP_VALUE(pllcfgr);
    const uint32_t pllq = PLLCFGR_PLLQ_VALUE(pllcfgr);

    if (pllq >= PLLCFGR_PLLQ_MIN && pllq <= PLLCFGR_PLLQ_MAX)
      return pllFrequency * ((pllp + 1) * 2) / pllq;
  }

  return 0;
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
static enum Result apb2ClockEnable(const void *,
    const void *configBase)
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
  const struct MainClockConfig * const config = configBase;
  assert(config != NULL);
  assert(config->divisor);
  assert(config->range <= VR_2V7_3V6);

  const unsigned int prescaler = 31 - countLeadingZeros32(config->divisor);
  assert((1 << prescaler) == config->divisor);
  assert(prescaler != 5 && prescaler <= 9);

  uint32_t cfgr = STM_RCC->CFGR & ~CFGR_HPRE_MASK;

  if (prescaler > 5)
    cfgr |= CFGR_HPRE(0x08 | (prescaler - 2));
  else if (prescaler > 0)
    cfgr |= CFGR_HPRE(0x08 | (prescaler - 1));

  volRange = config->range;
  updateAhbClock(cfgr, volRange);

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static uint32_t mainClockFrequency(const void *)
{
  const uint32_t divisor = ahbPrescalerToValue(CFGR_HPRE_VALUE(STM_RCC->CFGR));
  return systemClockFrequency(NULL) / divisor;
}
