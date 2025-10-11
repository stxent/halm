/*
 * clocking.c
 * Copyright (C) 2025 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/delay.h>
#include <halm/platform/lpc/clocking.h>
#include <halm/platform/lpc/gen_2/clocking_defs.h>
#include <halm/platform/lpc/system.h>
#include <halm/platform/platform_defs.h>
#include <xcore/accel.h>
#include <assert.h>
/*----------------------------------------------------------------------------*/
struct GenericClockClass
{
  struct ClockClass base;
  enum ClockBranch branch;
};
/*----------------------------------------------------------------------------*/
static uint32_t branchClockFrequency(int);
static struct ClockDescriptor *calcBranchDescriptor(enum ClockBranch);
static uint32_t calcPllFrequency(uint16_t, uint8_t, enum ClockSource);
static uint32_t calcPllValues(uint16_t, uint8_t);
/*----------------------------------------------------------------------------*/
static void extOscDisable(const void *);
static enum Result extOscEnable(const void *, const void *);
static uint32_t extOscFrequency(const void *);
static bool extOscReady(const void *);

static void intOscDisable(const void *);
static enum Result intOscEnable(const void *, const void *);
static uint32_t intOscFrequency(const void *);
static bool intOscReady(const void *);

#ifdef LPC_LPO_CLOCK
static void lpOscDisable(const void *);
static enum Result lpOscEnable(const void *, const void *);
static uint32_t lpOscFrequency(const void *);
static bool lpOscReady(const void *);
#endif

static void wdtOscDisable(const void *);
static enum Result wdtOscEnable(const void *, const void *);
static uint32_t wdtOscFrequency(const void *);
static bool wdtOscReady(const void *);

static uint32_t systemClockFrequency(const void *);

static void sysPllDisable(const void *);
static enum Result sysPllEnable(const void *, const void *);
static uint32_t sysPllFrequency(const void *);
static bool sysPllReady(const void *);

#ifdef LPC_USB_CLOCK
static void usbPllDisable(const void *);
static enum Result usbPllEnable(const void *, const void *);
static uint32_t usbPllFrequency(const void *);
static bool usbPllReady(const void *);
#endif
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

#ifdef LPC_LPO_CLOCK
const struct ClockClass * const LowPowerOsc = &(const struct ClockClass){
    .disable = lpOscDisable,
    .enable = lpOscEnable,
    .frequency = lpOscFrequency,
    .ready = lpOscReady
};
#endif

const struct ClockClass * const WdtOsc = &(const struct ClockClass){
    .disable = wdtOscDisable,
    .enable = wdtOscEnable,
    .frequency = wdtOscFrequency,
    .ready = wdtOscReady
};

const struct ClockClass * const SystemClock = &(const struct ClockClass){
    .disable = NULL,
    .enable = NULL,
    .frequency = systemClockFrequency,
    .ready = NULL
};

const struct ClockClass * const SystemPll = &(const struct ClockClass){
    .disable = sysPllDisable,
    .enable = sysPllEnable,
    .frequency = sysPllFrequency,
    .ready = sysPllReady
};

#ifdef LPC_USB_CLOCK
const struct ClockClass * const UsbPll = &(const struct ClockClass){
    .disable = usbPllDisable,
    .enable = usbPllEnable,
    .frequency = usbPllFrequency,
    .ready = usbPllReady
};
#endif
/*----------------------------------------------------------------------------*/
const struct ClockClass * const ClockOutput =
    (const struct ClockClass *)&(const struct GenericClockClass){
    .base = {
        .disable = branchDisable,
        .enable = clockOutputEnable,
        .frequency = branchFrequency,
        .ready = branchReady
    },
    .branch = CLOCK_BRANCH_OUTPUT
};

const struct ClockClass * const MainClock =
    (const struct ClockClass *)&(const struct GenericClockClass){
    .base = {
        .disable = branchDisable,
        .enable = branchEnable,
        .frequency = branchFrequency,
        .ready = branchReady
    },
    .branch = CLOCK_BRANCH_MAIN
};

#ifdef LPC_USB_CLOCK
const struct ClockClass * const UsbClock =
    (const struct ClockClass *)&(const struct GenericClockClass){
    .base = {
        .disable = branchDisable,
        .enable = branchEnable,
        .frequency = branchFrequency,
        .ready = branchReady
    },
    .branch = CLOCK_BRANCH_USB
};
#endif

#ifdef LPC_WDT_CLOCK
const struct ClockClass * const WdtClock =
    (const struct ClockClass *)&(const struct GenericClockClass){
    .base = {
        .disable = branchDisable,
        .enable = branchEnable,
        .frequency = branchFrequency,
        .ready = branchReady
    },
    .branch = CLOCK_BRANCH_WDT
};
#endif
/*----------------------------------------------------------------------------*/
static const int8_t branchSourceMap[][4] = {
    [CLOCK_BRANCH_MAIN] = {
        CLOCK_INTERNAL, CLOCK_EXTERNAL, CLOCK_WDT, CLOCK_PLL
    },
    [CLOCK_BRANCH_OUTPUT] = {
        CLOCK_INTERNAL, CLOCK_EXTERNAL, CLOCK_WDT, CLOCK_MAIN
    },

#ifdef LPC_USB_CLOCK
    [CLOCK_BRANCH_USB] = {
        CLOCK_USB_PLL, CLOCK_MAIN, -1, -1
    },
#endif

#ifdef LPC_WDT_CLOCK
    [CLOCK_BRANCH_WDT] = {
        CLOCK_INTERNAL, CLOCK_MAIN, CLOCK_WDT, -1
    }
#endif
};

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
static uint32_t branchClockFrequency(int source)
{
  switch (source)
  {
    case CLOCK_INTERNAL:
      return intOscFrequency(NULL);

    case CLOCK_EXTERNAL:
      return extOscFrequency(NULL);

    case CLOCK_PLL:
      return sysPllFrequency(NULL);

#ifdef LPC_USB_CLOCK
    case CLOCK_USB_PLL:
      return usbPllFrequency(NULL);
#endif

    case CLOCK_WDT:
      return wdtOscFrequency(NULL);

    default:
      return 0;
  }
}
/*----------------------------------------------------------------------------*/
static struct ClockDescriptor *calcBranchDescriptor(enum ClockBranch branch)
{
  volatile uint32_t *base = NULL;

  switch (branch)
  {
    case CLOCK_BRANCH_MAIN:
      base = &LPC_SYSCON->MAINCLKSEL;
      break;

    case CLOCK_BRANCH_OUTPUT:
      base = &LPC_SYSCON->CLKOUTCLKSEL;
      break;

#ifdef LPC_USB_CLOCK
    case CLOCK_BRANCH_USB:
      base = &LPC_SYSCON->USBCLKSEL;
      break;
#endif

#ifdef LPC_WDT_CLOCK
    case CLOCK_BRANCH_WDT:
      base = &LPC_SYSCON->WDTCLKSEL;
      break;
#endif
  }

  return (struct ClockDescriptor *)base;
}
/*----------------------------------------------------------------------------*/
static uint32_t calcPllFrequency(uint16_t multiplier, uint8_t divisor,
    enum ClockSource source)
{
  uint32_t frequency;

  switch (source)
  {
    case CLOCK_INTERNAL:
      frequency = intOscFrequency(NULL);
      break;

    case CLOCK_EXTERNAL:
      frequency = extOscFrequency(NULL);
      break;

    default:
      frequency = 0;
      break;
  }

  const uint32_t cco = frequency * multiplier;
  
  if (cco >= 156000000 && cco <= 320000000)
    return cco / divisor;
  else
    return 0;
}
/*----------------------------------------------------------------------------*/
static uint32_t calcPllValues(uint16_t multiplier, uint8_t divisor)
{
#ifdef XCORE_ACCEL_CLZ
  /* Post divider ratio is 2 x P */
  const unsigned int psel = 31 - countLeadingZeros32(divisor);
  assert(psel < 5 && 1 << psel == divisor);
#else
  /* Post divider ratio is 2 x P */
  unsigned int psel = 1;

  while (psel < 5 && 1 << psel != divisor)
    ++psel;
  /* Check whether actual divisor value found */
  assert(psel != 5);
#endif

  const unsigned int msel = multiplier >> psel;
  assert(msel - 1 < 32 && msel << psel == multiplier);

  return PLLCTRL_MSEL(msel - 1) | PLLCTRL_PSEL(psel - 1);
}
/*----------------------------------------------------------------------------*/
static void extOscDisable(const void *)
{
  sysPowerDisable(PWR_SYSOSC);
  extFrequency = 0;
}
/*----------------------------------------------------------------------------*/
static enum Result extOscEnable(const void *, const void *configBase)
{
  const struct ExternalOscConfig * const config = configBase;
  assert(config != NULL);
  assert(config->frequency >= 1000000 && config->frequency <= 25000000);

  uint32_t buffer = 0;

  if (config->bypass)
    buffer |= SYSOSCCTRL_BYPASS;
  if (config->frequency > 15000000)
    buffer |= SYSOSCCTRL_FREQRANGE;

  /* Configure crystal oscillator pins */
  configCrystalPins(config->bypass);
  /* Power-up oscillator */
  sysPowerEnable(PWR_SYSOSC);

  LPC_SYSCON->SYSOSCCTRL = buffer;

  /* There is no status register so wait for 10 microseconds */
  udelay(10);

  LPC_SYSCON->SYSPLLCLKSEL = PLLCLKSEL_SYSOSC;
  /* Update PLL clock source */
  clockSourceUpdate((struct ClockDescriptor *)&LPC_SYSCON->SYSPLLCLKSEL);

  extFrequency = config->frequency;
  return E_OK;
}
/*----------------------------------------------------------------------------*/
static uint32_t extOscFrequency(const void *)
{
  return extFrequency;
}
/*----------------------------------------------------------------------------*/
static bool extOscReady(const void *)
{
  return extFrequency != 0;
}
/*----------------------------------------------------------------------------*/
static void intOscDisable(const void *)
{
  sysPowerDisable(PWR_IRCOUT);
  sysPowerDisable(PWR_IRC);
}
/*----------------------------------------------------------------------------*/
static enum Result intOscEnable(const void *, const void *)
{
  sysPowerEnable(PWR_IRC);
  sysPowerEnable(PWR_IRCOUT);
  return E_OK;
}
/*----------------------------------------------------------------------------*/
static uint32_t intOscFrequency(const void *)
{
  return sysPowerStatus(PWR_IRC) && sysPowerStatus(PWR_IRCOUT) ?
      INT_OSC_FREQUENCY : 0;
}
/*----------------------------------------------------------------------------*/
static bool intOscReady(const void *)
{
  return sysPowerStatus(PWR_IRC) && sysPowerStatus(PWR_IRCOUT);
}
/*----------------------------------------------------------------------------*/
#ifdef LPC_LPO_CLOCK
static void lpOscDisable(const void *)
{
  LPC_PMU->DPDCTRL &= ~(DPDCTRL_LPOSCEN | DPDCTRL_LPOSCDPDEN);
}
#endif
/*----------------------------------------------------------------------------*/
#ifdef LPC_LPO_CLOCK
static enum Result lpOscEnable(const void *, const void *)
{
  LPC_PMU->DPDCTRL |= DPDCTRL_LPOSCEN | DPDCTRL_LPOSCDPDEN;
  return E_OK;
}
#endif
/*----------------------------------------------------------------------------*/
#ifdef LPC_LPO_CLOCK
static uint32_t lpOscFrequency(const void *)
{
  return (LPC_PMU->DPDCTRL & DPDCTRL_LPOSCEN) ? LP_OSC_FREQUENCY : 0;
}
#endif
/*----------------------------------------------------------------------------*/
#ifdef LPC_LPO_CLOCK
static bool lpOscReady(const void *)
{
  return (LPC_PMU->DPDCTRL & DPDCTRL_LPOSCEN) != 0;
}
#endif
/*----------------------------------------------------------------------------*/
static void wdtOscDisable(const void *)
{
  sysPowerDisable(PWR_WDTOSC);
  wdtFrequency = 0;
}
/*----------------------------------------------------------------------------*/
static enum Result wdtOscEnable(const void *, const void *configBase)
{
  const struct WdtOscConfig * const config = configBase;
  assert(config != NULL);
  assert(config->frequency <= WDT_FREQ_4600);
  assert(config->divisor <= 64 && config->divisor % 2 == 0);

  const unsigned int divsel = config->divisor ?
      (config->divisor >> 1) - 1 : 0;
  const unsigned int freqsel = config->frequency != WDT_FREQ_DEFAULT ?
      config->frequency : WDT_FREQ_600;

  LPC_SYSCON->WDTOSCCTRL = WDTOSCCTRL_DIVSEL(divsel)
      | WDTOSCCTRL_FREQSEL(freqsel);
  sysPowerEnable(PWR_WDTOSC);

  wdtFrequency = (wdtFrequencyValues[freqsel - 1] * 1000) >> 1;
  return E_OK;
}
/*----------------------------------------------------------------------------*/
static uint32_t wdtOscFrequency(const void *)
{
  return wdtFrequency;
}
/*----------------------------------------------------------------------------*/
static bool wdtOscReady(const void *)
{
  return wdtFrequency != 0;
}
/*----------------------------------------------------------------------------*/
static uint32_t systemClockFrequency(const void *)
{
  const int source =
      branchSourceMap[CLOCK_BRANCH_MAIN][LPC_SYSCON->MAINCLKSEL];

  return branchClockFrequency(source);
}
/*----------------------------------------------------------------------------*/
static void sysPllDisable(const void *)
{
  sysPowerDisable(PWR_SYSPLL);
}
/*----------------------------------------------------------------------------*/
static enum Result sysPllEnable(const void *, const void *configBase)
{
  const struct PllConfig * const config = configBase;
  assert(config != NULL);
  assert(config->source == CLOCK_EXTERNAL || config->source == CLOCK_INTERNAL);

  const uint32_t control = calcPllValues(config->multiplier, config->divisor);
  const uint32_t frequency = calcPllFrequency(config->multiplier,
      config->divisor, config->source);

  if (!frequency)
    return E_VALUE;

  /* Power-up PLL */
  sysPowerEnable(PWR_SYSPLL);

  /* Select clock source */
  LPC_SYSCON->SYSPLLCLKSEL = config->source == CLOCK_EXTERNAL ?
      PLLCLKSEL_SYSOSC : PLLCLKSEL_IRC;
  /* Update clock source for changes to take effect */
  clockSourceUpdate((struct ClockDescriptor *)&LPC_SYSCON->SYSPLLCLKSEL);
  /* Set feedback divider value and post divider ratio */
  LPC_SYSCON->SYSPLLCTRL = control;

  pllFrequency = frequency;
  return E_OK;
}
/*----------------------------------------------------------------------------*/
static uint32_t sysPllFrequency(const void *)
{
  return (LPC_SYSCON->SYSPLLSTAT & PLLSTAT_LOCK) ? pllFrequency : 0;
}
/*----------------------------------------------------------------------------*/
static bool sysPllReady(const void *)
{
  return pllFrequency && (LPC_SYSCON->SYSPLLSTAT & PLLSTAT_LOCK);
}
/*----------------------------------------------------------------------------*/
#ifdef LPC_USB_CLOCK
static void usbPllDisable(const void *)
{
  sysPowerDisable(PWR_USBPLL);
}
#endif
/*----------------------------------------------------------------------------*/
#ifdef LPC_USB_CLOCK
static enum Result usbPllEnable(const void *, const void *configBase)
{
  const struct PllConfig * const config = configBase;
  assert(config != NULL);
  assert(config->source == CLOCK_EXTERNAL);

  const uint32_t control = calcPllValues(config->multiplier, config->divisor);
  const uint32_t frequency = calcPllFrequency(config->multiplier,
      config->divisor, CLOCK_EXTERNAL);
  
  if (frequency != USB_PLL_FREQUENCY)
    return E_VALUE;

  /* Power-up PLL */
  sysPowerEnable(PWR_USBPLL);

  /*
   * The USB PLL clock source must be switched to system oscillator
   * for correct operation.
   */
  LPC_SYSCON->USBPLLCLKSEL = PLLCLKSEL_SYSOSC;
  /* Update clock source for changes to take effect */
  clockSourceUpdate((struct ClockDescriptor *)&LPC_SYSCON->USBPLLCLKSEL);
  /* Set feedback divider value and post divider ratio */
  LPC_SYSCON->USBPLLCTRL = control;

  return E_OK;
}
#endif
/*----------------------------------------------------------------------------*/
#ifdef LPC_USB_CLOCK
static uint32_t usbPllFrequency(const void *)
{
  return (LPC_SYSCON->USBPLLSTAT & PLLSTAT_LOCK) ? USB_PLL_FREQUENCY : 0;
}
#endif
/*----------------------------------------------------------------------------*/
#ifdef LPC_USB_CLOCK
static bool usbPllReady(const void *)
{
  return (LPC_SYSCON->USBPLLSTAT & PLLSTAT_LOCK) != 0;
}
#endif
/*----------------------------------------------------------------------------*/
static enum Result clockOutputEnable(const void *, const void *configBase)
{
  const struct ClockOutputConfig * const config = configBase;
  assert(config != NULL);

  const struct GenericClockConfig baseConfig = {
      .source = config->source,
      .divisor = config->divisor
  };

  configClockOutput(config->pin);
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
  const struct GenericClockConfig * const config = configBase;
  assert(config != NULL);

  const struct GenericClockClass * const clock = clockBase;
  struct ClockDescriptor * const descriptor =
      calcBranchDescriptor(clock->branch);
  int source = -1;

  for (unsigned int index = 0; index < 4; ++index)
  {
    if (branchSourceMap[clock->branch][index] == config->source)
    {
      source = index;
      break;
    }
  }
  assert(source != -1);

  if (clock->branch == CLOCK_BRANCH_MAIN)
  {
    /* Max PLL output frequency is 100 MHz, set safe divisor value */
    descriptor->divider = 5;
    /* Set safe latency value */
    sysFlashLatencyReset();
  }

  descriptor->sourceSelect = (uint32_t)source;
  /* Update clock source */
  clockSourceUpdate(descriptor);
  /* Enable clock */
  descriptor->divider = config->divisor ? config->divisor : 1;

  if (clock->branch == CLOCK_BRANCH_MAIN)
  {
    const uint32_t frequency = branchFrequency(MainClock);
    const unsigned int latency = sysFlashLatencyFromFrequency(frequency);

    sysFlashLatencyUpdate(latency);
    ticksPerSecond = TICK_RATE(frequency, latency);
  }

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static uint32_t branchFrequency(const void *clockBase)
{
  const struct GenericClockClass * const clock = clockBase;
  const struct ClockDescriptor * const descriptor =
      calcBranchDescriptor(clock->branch);
  const int source =
      branchSourceMap[clock->branch][descriptor->sourceSelect];

  if (!descriptor->divider || source == -1)
    return 0;

  uint32_t baseFrequency;

  if (source == CLOCK_MAIN)
    baseFrequency = systemClockFrequency(NULL);
  else
    baseFrequency = branchClockFrequency(source);

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
