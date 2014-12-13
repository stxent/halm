/*
 * clocking.c
 * Copyright (C) 2013 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <delay.h>
#include <platform/platform_defs.h>
#include <platform/nxp/lpc11xx/clocking.h>
#include <platform/nxp/lpc11xx/clocking_defs.h>
#include <platform/nxp/lpc11xx/system.h>
/*----------------------------------------------------------------------------*/
#define INT_OSC_FREQUENCY               12000000
#define TICK_RATE(frequency, latency)   ((frequency) / (latency) / 1000)
/*----------------------------------------------------------------------------*/
struct ClockDescriptor
{
  volatile uint32_t sourceSelect;
  volatile uint32_t sourceUpdate;
  volatile uint32_t divider;
};
/*----------------------------------------------------------------------------*/
static struct ClockDescriptor *calcBranchDescriptor(enum clockBranch);
static uint32_t calcPllFrequency(uint16_t, uint8_t, enum clockSource);
static enum result calcPllValues(uint16_t, uint8_t, uint32_t *);
static inline void flashLatencyReset(void);
static void flashLatencyUpdate(uint32_t);
/*----------------------------------------------------------------------------*/
static void extOscDisable(const void *);
static enum result extOscEnable(const void *, const void *);
static uint32_t extOscFrequency(const void *);
static bool extOscReady(const void *);

static void intOscDisable(const void *);
static enum result intOscEnable(const void *, const void *);
static uint32_t intOscFrequency(const void *);
static bool intOscReady(const void *);

static void wdtOscDisable(const void *);
static enum result wdtOscEnable(const void *, const void *);
static uint32_t wdtOscFrequency(const void *);
static bool wdtOscReady(const void *);

static void sysPllDisable(const void *);
static enum result sysPllEnable(const void *, const void *);
static uint32_t sysPllFrequency(const void *);
static bool sysPllReady(const void *);
/*----------------------------------------------------------------------------*/
static void branchDisable(const void *);
static enum result branchEnable(const void *, const void *);
static uint32_t branchFrequency(const void *);
static bool branchReady(const void *);
/*----------------------------------------------------------------------------*/
static const struct ClockClass extOscTable = {
    .disable = extOscDisable,
    .enable = extOscEnable,
    .frequency = extOscFrequency,
    .ready = extOscReady
};

static const struct ClockClass intOscTable = {
    .disable = intOscDisable,
    .enable = intOscEnable,
    .frequency = intOscFrequency,
    .ready = intOscReady
};

static const struct ClockClass wdtOscTable = {
    .disable = wdtOscDisable,
    .enable = wdtOscEnable,
    .frequency = wdtOscFrequency,
    .ready = wdtOscReady
};

static const struct ClockClass sysPllTable = {
    .disable = sysPllDisable,
    .enable = sysPllEnable,
    .frequency = sysPllFrequency,
    .ready = sysPllReady
};
/*----------------------------------------------------------------------------*/
static const struct CommonClockClass clockOutputTable = {
    .parent = {
        .disable = branchDisable,
        .enable = branchEnable,
        .frequency = branchFrequency,
        .ready = branchReady,
    },
    .branch = CLOCK_BRANCH_OUTPUT
};

static const struct CommonClockClass mainClockTable = {
    .parent = {
        .disable = branchDisable,
        .enable = branchEnable,
        .frequency = branchFrequency,
        .ready = branchReady,
    },
    .branch = CLOCK_BRANCH_MAIN
};

static const struct CommonClockClass wdtClockTable = {
    .parent = {
        .disable = branchDisable,
        .enable = branchEnable,
        .frequency = branchFrequency,
        .ready = branchReady,
    },
    .branch = CLOCK_BRANCH_WDT
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
static const uint16_t wdtFrequencyValues[15] = {
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
const struct ClockClass * const ExternalOsc = &extOscTable;
const struct ClockClass * const InternalOsc = &intOscTable;
const struct ClockClass * const WdtOsc = &wdtOscTable;
const struct ClockClass * const SystemPll = &sysPllTable;
const struct CommonClockClass * const ClockOutput = &clockOutputTable;
const struct CommonClockClass * const MainClock = &mainClockTable;
const struct CommonClockClass * const WdtClock = &wdtClockTable;
/*----------------------------------------------------------------------------*/
static uint32_t extFrequency = 0;
static uint32_t pllFrequency = 0;
static uint32_t wdtFrequency = 0;
uint32_t ticksPerSecond = TICK_RATE(INT_OSC_FREQUENCY, 3);
/*----------------------------------------------------------------------------*/
static struct ClockDescriptor *calcBranchDescriptor(enum clockBranch branch)
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
    enum clockSource source)
{
  uint32_t frequency;

  switch (source)
  {
    case CLOCK_EXTERNAL:
      frequency = extFrequency;
      break;

    case CLOCK_INTERNAL:
      frequency = INT_OSC_FREQUENCY;
      break;

    default:
      return 0;
  }

  /* Check CCO range */
  frequency = frequency * multiplier;
  if (frequency < 156000000 || frequency > 320000000)
    return 0;

  return frequency / divisor;
}
/*----------------------------------------------------------------------------*/
static enum result calcPllValues(uint16_t multiplier, uint8_t divisor,
    uint32_t *result)
{
  if (!multiplier || !divisor || divisor & 1)
    return E_VALUE;

  const uint8_t msel = multiplier / divisor - 1;
  uint8_t counter = 0;
  uint8_t psel = divisor >> 1;

  if (msel >= 32)
    return E_VALUE;

  while (counter < 4 && psel != 1 << counter)
    counter++;
  /* Check whether actual divisor value found */
  if ((psel = counter) == 4)
    return E_VALUE;

  *result = PLLCTRL_MSEL(msel) | PLLCTRL_PSEL(psel);
  return E_OK;
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
  const uint8_t clocks = 1 + frequency / 20000000;

  sysFlashLatencyUpdate(clocks <= 3 ? clocks : 3);
}
/*----------------------------------------------------------------------------*/
static void extOscDisable(const void *clockBase __attribute__((unused)))
{
  sysPowerDisable(PWR_SYSOSC);
  extFrequency = 0;
}
/*----------------------------------------------------------------------------*/
static enum result extOscEnable(const void *clockBase
    __attribute__((unused)), const void *configBase)
{
  const struct ExternalOscConfig * const config = configBase;
  uint32_t buffer = 0;

  if (config->bypass)
    buffer |= SYSOSCCTRL_BYPASS;
  if (config->frequency > 15000000)
    buffer |= SYSOSCCTRL_FREQRANGE;

  LPC_SYSCON->SYSOSCCTRL = buffer;
  sysPowerEnable(PWR_SYSOSC);

  /* There is no status register so wait for 10 microseconds */
  udelay(10);

  extFrequency = config->frequency;

  LPC_SYSCON->SYSPLLCLKSEL = PLLCLKSEL_SYSOSC;
  /* Update PLL clock source */
  LPC_SYSCON->SYSPLLCLKUEN = CLKUEN_ENA;
  LPC_SYSCON->SYSPLLCLKUEN = 0;
  LPC_SYSCON->SYSPLLCLKUEN = CLKUEN_ENA;
  /* Wait until updated */
  while (!(LPC_SYSCON->SYSPLLCLKUEN & CLKUEN_ENA));

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
static enum result intOscEnable(const void *clockBase __attribute__((unused)),
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
static enum result wdtOscEnable(const void *clockBase __attribute__((unused)),
    const void *configBase)
{
  const struct WdtOscConfig * const config = configBase;

  if (config->frequency > WDT_FREQ_4600)
    return E_VALUE;

  const uint8_t index = !config->frequency ? WDT_FREQ_600 : config->frequency;

  sysPowerEnable(PWR_WDTOSC);
  LPC_SYSCON->WDTOSCCTRL = WDTOSCCTRL_DIVSEL(0) | WDTOSCCTRL_FREQSEL(index);
  wdtFrequency = ((uint32_t)wdtFrequencyValues[index - 1] * 1000) >> 1;

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
static enum result sysPllEnable(const void *clockBase __attribute__((unused)),
    const void *configBase)
{
  const struct PllConfig * const config = configBase;
  uint32_t control; /* Control register value */

  const enum result res = calcPllValues(config->multiplier,
      config->divisor, &control);
  if (res != E_OK)
    return res;

  const uint32_t frequency = calcPllFrequency(config->multiplier,
      config->divisor, config->source);
  if (!frequency)
    return E_VALUE;

  pllFrequency = frequency;

  /* Select clock source */
  LPC_SYSCON->SYSPLLCLKSEL = config->source == CLOCK_EXTERNAL ?
      PLLCLKSEL_SYSOSC : PLLCLKSEL_IRC;

  /* Update clock source for changes to take effect */
  LPC_SYSCON->SYSPLLCLKUEN = CLKUEN_ENA;
  LPC_SYSCON->SYSPLLCLKUEN = 0;
  LPC_SYSCON->SYSPLLCLKUEN = CLKUEN_ENA;
  /* Wait until updated */
  while (!(LPC_SYSCON->SYSPLLCLKUEN & CLKUEN_ENA));

  LPC_SYSCON->SYSPLLCTRL = control;
  sysPowerEnable(PWR_SYSPLL);

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
static void branchDisable(const void *clockBase)
{
  const struct CommonClockClass * const clock = clockBase;
  struct ClockDescriptor * const descriptor =
      calcBranchDescriptor(clock->branch);

  if (clock->branch != CLOCK_BRANCH_MAIN)
    descriptor->divider = 0;
}
/*----------------------------------------------------------------------------*/
static enum result branchEnable(const void *clockBase, const void *configBase)
{
  const struct CommonClockClass * const clock = clockBase;
  const struct CommonClockConfig * const config = configBase;
  struct ClockDescriptor * const descriptor =
      calcBranchDescriptor(clock->branch);
  int8_t value = -1;

  for (uint8_t index = 0; index < 4; ++index)
  {
    if (commonClockSourceMap[clock->branch][index] == (int8_t)config->source)
    {
      value = index;
      break;
    }
  }

  if (value == -1)
    return E_VALUE;

  if (clock->branch == CLOCK_BRANCH_MAIN)
    flashLatencyReset();

  descriptor->sourceSelect = (uint32_t)value;

  /* Update clock source */
  descriptor->sourceUpdate = CLKUEN_ENA;
  descriptor->sourceUpdate = 0;
  descriptor->sourceUpdate = CLKUEN_ENA;
  /* Wait until updated */
  while (!(descriptor->sourceUpdate & CLKUEN_ENA));

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
  const struct CommonClockClass * const clock = clockBase;
  const struct ClockDescriptor * const descriptor =
      calcBranchDescriptor(clock->branch);
  const int8_t sourceType =
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
  const struct CommonClockClass * const clock = clockBase;
  const struct ClockDescriptor * const descriptor =
      calcBranchDescriptor(clock->branch);

  return descriptor->divider != 0;
}
