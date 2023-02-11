/*
 * clocking.c
 * Copyright (C) 2013 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/delay.h>
#include <halm/platform/lpc/clocking.h>
#include <halm/platform/lpc/lpc11xx/clocking_defs.h>
#include <halm/platform/lpc/system.h>
#include <halm/platform/platform_defs.h>
#include <assert.h>
/*----------------------------------------------------------------------------*/
#define INT_OSC_FREQUENCY             12000000
#define TICK_RATE(frequency, latency) ((frequency) / (latency) / 1000)
/*----------------------------------------------------------------------------*/
struct ClockDescriptor
{
  volatile uint32_t sourceSelect;
  volatile uint32_t sourceUpdate;
  volatile uint32_t divider;
};

struct GenericClockClass
{
  struct ClockClass base;
  enum ClockBranch branch;
};
/*----------------------------------------------------------------------------*/
static struct ClockDescriptor *calcBranchDescriptor(enum ClockBranch);
static uint32_t calcPllFrequency(uint16_t, uint8_t, enum ClockSource);
static uint32_t calcPllValues(uint16_t, uint8_t);
static inline void flashLatencyReset(void);
static void flashLatencyUpdate(uint32_t);
/*----------------------------------------------------------------------------*/
static void extOscDisable(const void *);
static enum Result extOscEnable(const void *, const void *);
static uint32_t extOscFrequency(const void *);
static bool extOscReady(const void *);

static void intOscDisable(const void *);
static enum Result intOscEnable(const void *, const void *);
static uint32_t intOscFrequency(const void *);
static bool intOscReady(const void *);

static void wdtOscDisable(const void *);
static enum Result wdtOscEnable(const void *, const void *);
static uint32_t wdtOscFrequency(const void *);
static bool wdtOscReady(const void *);

static void sysPllDisable(const void *);
static enum Result sysPllEnable(const void *, const void *);
static uint32_t sysPllFrequency(const void *);
static bool sysPllReady(const void *);
/*----------------------------------------------------------------------------*/
static enum Result clockOutputEnable(const void *, const void *);

static void branchDisable(const void *);
static enum Result branchEnable(const void *, const void *);
static uint32_t branchFrequency(const void *);
static bool branchReady(const void *);
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

const struct ClockClass * const WdtOsc = &(const struct ClockClass){
    .disable = wdtOscDisable,
    .enable = wdtOscEnable,
    .frequency = wdtOscFrequency,
    .ready = wdtOscReady
};

const struct ClockClass * const SystemPll = &(const struct ClockClass){
    .disable = sysPllDisable,
    .enable = sysPllEnable,
    .frequency = sysPllFrequency,
    .ready = sysPllReady
};
/*----------------------------------------------------------------------------*/
const struct ClockClass * const ClockOutput =
    (const struct ClockClass *)&(const struct GenericClockClass){
    .base = {
        .disable = branchDisable,
        .enable = clockOutputEnable,
        .frequency = branchFrequency,
        .ready = branchReady,
    },
    .branch = CLOCK_BRANCH_OUTPUT
};

const struct ClockClass * const MainClock =
    (const struct ClockClass *)&(const struct GenericClockClass){
    .base = {
        .disable = branchDisable,
        .enable = branchEnable,
        .frequency = branchFrequency,
        .ready = branchReady,
    },
    .branch = CLOCK_BRANCH_MAIN
};

const struct ClockClass * const WdtClock =
    (const struct ClockClass *)&(const struct GenericClockClass){
    .base = {
        .disable = branchDisable,
        .enable = branchEnable,
        .frequency = branchFrequency,
        .ready = branchReady,
    },
    .branch = CLOCK_BRANCH_WDT
};
/*----------------------------------------------------------------------------*/
static const struct PinEntry clockOutputPins[] = {
    {
        .key = PIN(0, 1), /* CLKOUT */
        .channel = 0,
        .value = 1
    }, {
        .key = 0 /* End of pin function association list */
    }
};
/*----------------------------------------------------------------------------*/
static const int8_t commonClockSourceMap[4][4] = {
    [CLOCK_BRANCH_MAIN] = {
        CLOCK_INTERNAL, CLOCK_EXTERNAL, CLOCK_WDT, CLOCK_PLL
    },
    [CLOCK_BRANCH_OUTPUT] = {
        CLOCK_INTERNAL, CLOCK_EXTERNAL, CLOCK_WDT, CLOCK_MAIN
    },
    [CLOCK_BRANCH_WDT] = {
        CLOCK_INTERNAL, CLOCK_MAIN, CLOCK_WDT, -1
    }
};
/*----------------------------------------------------------------------------*/
static const uint16_t wdtFrequencyValues[] = {
    600,
    1050,
    1400,
    1750,
    2100,
    2400,
    2700,
    3000,
    3250,
    3500,
    3750,
    4000,
    4200,
    4400,
    4600
};
/*----------------------------------------------------------------------------*/
static uint32_t extFrequency = 0;
static uint32_t pllFrequency = 0;
static uint32_t wdtFrequency = 0;
uint32_t ticksPerSecond = TICK_RATE(INT_OSC_FREQUENCY, 3);
/*----------------------------------------------------------------------------*/
static struct ClockDescriptor *calcBranchDescriptor(enum ClockBranch branch)
{
  volatile uint32_t *base = 0;

  switch (branch)
  {
    case CLOCK_BRANCH_MAIN:
      base = &LPC_SYSCON->MAINCLKSEL;
      break;

    case CLOCK_BRANCH_OUTPUT:
      base = &LPC_SYSCON->CLKOUTCLKSEL;
      break;

    case CLOCK_BRANCH_WDT:
      base = &LPC_SYSCON->WDTCLKSEL;
      break;
  }

  return (struct ClockDescriptor *)base;
}
/*----------------------------------------------------------------------------*/
static uint32_t calcPllFrequency(uint16_t multiplier, uint8_t divisor,
    enum ClockSource source)
{
  assert(source == CLOCK_EXTERNAL || source == CLOCK_INTERNAL);

  uint32_t frequency = 0;

  switch (source)
  {
    case CLOCK_EXTERNAL:
      frequency = extFrequency;
      break;

    case CLOCK_INTERNAL:
      frequency = INT_OSC_FREQUENCY;
      break;

    default:
      break;
  }

  /* Check CCO range */
  frequency = frequency * multiplier;
  assert(frequency >= 156000000 && frequency <= 320000000);

  return frequency / divisor;
}
/*----------------------------------------------------------------------------*/
static uint32_t calcPllValues(uint16_t multiplier, uint8_t divisor)
{
  assert(multiplier);
  assert(divisor % 2 == 0);

  const unsigned int msel = multiplier / divisor - 1;
  unsigned int psel = 0;
  unsigned int sourceDivisor = divisor >> 1;

  assert(msel < 32);

  while (psel < 4 && sourceDivisor != 1U << psel)
    ++psel;
  /* Check whether actual divisor value found */
  assert(psel != 4);

  return PLLCTRL_MSEL(msel) | PLLCTRL_PSEL(psel);
}
/*----------------------------------------------------------------------------*/
static inline void flashLatencyReset(void)
{
  /* Set safe latency settings */
  sysFlashLatencyUpdate(3);
}
/*----------------------------------------------------------------------------*/
static void flashLatencyUpdate(uint32_t frequency)
{
  static const uint32_t frequencyStep = 20000000;
  const unsigned int clocks = (frequency + (frequencyStep - 1)) / frequencyStep;

  sysFlashLatencyUpdate(MIN(clocks, 3));
}
/*----------------------------------------------------------------------------*/
static void extOscDisable(const void *clockBase __attribute__((unused)))
{
  sysPowerDisable(PWR_SYSOSC);
  extFrequency = 0;
}
/*----------------------------------------------------------------------------*/
static enum Result extOscEnable(const void *clockBase
    __attribute__((unused)), const void *configBase)
{
  const struct ExternalOscConfig * const config = configBase;
  uint32_t buffer = 0;

  assert(config->frequency >= 1000000 && config->frequency <= 25000000);

  if (config->bypass)
    buffer |= SYSOSCCTRL_BYPASS;
  if (config->frequency > 15000000)
    buffer |= SYSOSCCTRL_FREQRANGE;

  /* Power-up oscillator */
  sysPowerEnable(PWR_SYSOSC);

  LPC_SYSCON->SYSOSCCTRL = buffer;

  /* There is no status register so wait for 10 microseconds */
  udelay(10);

  LPC_SYSCON->SYSPLLCLKSEL = PLLCLKSEL_SYSOSC;
  /* Update PLL clock source */
  LPC_SYSCON->SYSPLLCLKUEN = 0;
  LPC_SYSCON->SYSPLLCLKUEN = CLKUEN_ENA;

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
  return extFrequency != 0;
}
/*----------------------------------------------------------------------------*/
static void intOscDisable(const void *clockBase __attribute__((unused)))
{
  sysPowerDisable(PWR_IRCOUT);
  sysPowerDisable(PWR_IRC);
}
/*----------------------------------------------------------------------------*/
static enum Result intOscEnable(const void *clockBase __attribute__((unused)),
    const void *configBase __attribute__((unused)))
{
  sysPowerEnable(PWR_IRC);
  sysPowerEnable(PWR_IRCOUT);
  return E_OK;
}
/*----------------------------------------------------------------------------*/
static uint32_t intOscFrequency(const void *clockBase __attribute__((unused)))
{
  return sysPowerStatus(PWR_IRC) && sysPowerStatus(PWR_IRCOUT) ?
      INT_OSC_FREQUENCY : 0;
}
/*----------------------------------------------------------------------------*/
static bool intOscReady(const void *clockBase __attribute__((unused)))
{
  return sysPowerStatus(PWR_IRC) && sysPowerStatus(PWR_IRCOUT);
}
/*----------------------------------------------------------------------------*/
static void wdtOscDisable(const void *clockBase __attribute__((unused)))
{
  sysPowerDisable(PWR_WDTOSC);
  wdtFrequency = 0;
}
/*----------------------------------------------------------------------------*/
static enum Result wdtOscEnable(const void *clockBase __attribute__((unused)),
    const void *configBase)
{
  const struct WdtOscConfig * const config = configBase;

  assert(config->frequency <= WDT_FREQ_4600);

  const unsigned int index = !config->frequency ?
      WDT_FREQ_600 : config->frequency;

  sysPowerEnable(PWR_WDTOSC);
  LPC_SYSCON->WDTOSCCTRL = WDTOSCCTRL_DIVSEL(0) | WDTOSCCTRL_FREQSEL(index);
  wdtFrequency = (wdtFrequencyValues[index - 1] * 1000) >> 1;

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static uint32_t wdtOscFrequency(const void *clockBase __attribute__((unused)))
{
  return wdtFrequency;
}
/*----------------------------------------------------------------------------*/
static bool wdtOscReady(const void *clockBase __attribute__((unused)))
{
  return wdtFrequency != 0;
}
/*----------------------------------------------------------------------------*/
static void sysPllDisable(const void *clockBase __attribute__((unused)))
{
  sysPowerDisable(PWR_SYSPLL);
  pllFrequency = 0;
}
/*----------------------------------------------------------------------------*/
static enum Result sysPllEnable(const void *clockBase __attribute__((unused)),
    const void *configBase)
{
  const struct PllConfig * const config = configBase;

  assert(config->divisor);
  assert(config->source == CLOCK_EXTERNAL || config->source == CLOCK_INTERNAL);

  const uint32_t control = calcPllValues(config->multiplier,
      config->divisor);
  const uint32_t frequency = calcPllFrequency(config->multiplier,
      config->divisor, config->source);

  /* Power-up PLL */
  sysPowerEnable(PWR_SYSPLL);

  /* Select clock source */
  LPC_SYSCON->SYSPLLCLKSEL = config->source == CLOCK_EXTERNAL ?
      PLLCLKSEL_SYSOSC : PLLCLKSEL_IRC;
  /* Update clock source for changes to take effect */
  LPC_SYSCON->SYSPLLCLKUEN = 0;
  LPC_SYSCON->SYSPLLCLKUEN = CLKUEN_ENA;
  /* Set feedback divider value and post divider ratio */
  LPC_SYSCON->SYSPLLCTRL = control;

  pllFrequency = frequency;

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static uint32_t sysPllFrequency(const void *clockBase __attribute__((unused)))
{
  return pllFrequency;
}
/*----------------------------------------------------------------------------*/
static bool sysPllReady(const void *clockBase __attribute__((unused)))
{
  return pllFrequency && (LPC_SYSCON->SYSPLLSTAT & PLLSTAT_LOCK);
}
/*----------------------------------------------------------------------------*/
static enum Result clockOutputEnable(const void *clockBase
    __attribute__((unused)), const void *configBase)
{
  const struct ClockOutputConfig * const config = configBase;
  const struct GenericClockConfig baseConfig = {
      .source = config->source,
      .divisor = config->divisor
  };

  const struct PinEntry * const pinEntry = pinFind(clockOutputPins,
      config->pin, 0);
  assert(pinEntry);

  const struct Pin pin = pinInit(config->pin);

  pinInput(pin);
  pinSetFunction(pin, pinEntry->value);

  return branchEnable(ClockOutput, &baseConfig);
}
/*----------------------------------------------------------------------------*/
static void branchDisable(const void *clockBase)
{
  const struct GenericClockClass * const clock = clockBase;
  struct ClockDescriptor * const descriptor =
      calcBranchDescriptor(clock->branch);

  if (clock->branch != CLOCK_BRANCH_MAIN)
    descriptor->divider = 0;
}
/*----------------------------------------------------------------------------*/
static enum Result branchEnable(const void *clockBase, const void *configBase)
{
  const struct GenericClockClass * const clock = clockBase;
  const struct GenericClockConfig * const config = configBase;
  struct ClockDescriptor * const descriptor =
      calcBranchDescriptor(clock->branch);
  int source = -1;

  for (unsigned int index = 0; index < 4; ++index)
  {
    if (commonClockSourceMap[clock->branch][index] == config->source)
    {
      source = index;
      break;
    }
  }
  assert(source != -1);

  if (clock->branch == CLOCK_BRANCH_MAIN)
    flashLatencyReset();

  descriptor->sourceSelect = (uint32_t)source;

  /* Update clock source */
  descriptor->sourceUpdate = 0;
  descriptor->sourceUpdate = CLKUEN_ENA;

  /* Enable clock */
  descriptor->divider = config->divisor ? config->divisor : 1;

  if (clock->branch == CLOCK_BRANCH_MAIN)
  {
    const uint32_t frequency = branchFrequency(MainClock);

    flashLatencyUpdate(frequency);
    ticksPerSecond = TICK_RATE(frequency, sysFlashLatency());
  }

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static uint32_t branchFrequency(const void *clockBase)
{
  const struct GenericClockClass * const clock = clockBase;
  const struct ClockDescriptor * const descriptor =
      calcBranchDescriptor(clock->branch);
  const int sourceType =
      commonClockSourceMap[clock->branch][descriptor->sourceSelect];

  if (!descriptor->divider || sourceType == -1)
    return 0;

  uint32_t baseFrequency = 0;

  switch (sourceType)
  {
    case CLOCK_INTERNAL:
      baseFrequency = intOscFrequency(0);
      break;

    case CLOCK_EXTERNAL:
      baseFrequency = extOscFrequency(0);
      break;

    case CLOCK_PLL:
      baseFrequency = sysPllFrequency(0);
      break;

    case CLOCK_WDT:
      baseFrequency = wdtOscFrequency(0);
      break;

    case CLOCK_MAIN:
      baseFrequency = branchFrequency(MainClock);
      break;
  }

  return baseFrequency / descriptor->divider;
}
/*----------------------------------------------------------------------------*/
static bool branchReady(const void *clockBase)
{
  const struct GenericClockClass * const clock = clockBase;
  const struct ClockDescriptor * const descriptor =
      calcBranchDescriptor(clock->branch);

  return descriptor->divider != 0;
}
