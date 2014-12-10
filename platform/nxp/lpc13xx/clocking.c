/*
 * clocking.c
 * Copyright (C) 2013 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <delay.h>
#include <platform/platform_defs.h>
#include <platform/nxp/lpc13xx/clocking.h>
#include <platform/nxp/lpc13xx/clocking_defs.h>
#include <platform/nxp/lpc13xx/system.h>
/*----------------------------------------------------------------------------*/
#define INT_OSC_FREQUENCY 12000000
#define USB_FREQUENCY     48000000
/*----------------------------------------------------------------------------*/
static uint32_t calcPllFrequency(uint16_t, uint8_t, enum clockSource);
static enum result calcPllValues(uint16_t, uint8_t, uint32_t *);
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

static void usbPllDisable(const void *);
static enum result usbPllEnable(const void *, const void *);
static uint32_t usbPllFrequency(const void *);
static bool usbPllReady(const void *);
/*----------------------------------------------------------------------------*/
static void clockOutputDisable(const void *);
static enum result clockOutputEnable(const void *, const void *);
static uint32_t clockOutputFrequency(const void *);
static bool clockOutputReady(const void *);

static void mainClockDisable(const void *);
static enum result mainClockEnable(const void *, const void *);
static uint32_t mainClockFrequency(const void *);
static bool mainClockReady(const void *);

static void usbClockDisable(const void *);
static enum result usbClockEnable(const void *, const void *);
static uint32_t usbClockFrequency(const void *);
static bool usbClockReady(const void *);

static void wdtClockDisable(const void *);
static enum result wdtClockEnable(const void *, const void *);
static uint32_t wdtClockFrequency(const void *);
static bool wdtClockReady(const void *);
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

static const struct ClockClass usbPllTable = {
    .disable = usbPllDisable,
    .enable = usbPllEnable,
    .frequency = usbPllFrequency,
    .ready = usbPllReady
};
/*----------------------------------------------------------------------------*/
static const struct ClockClass clockOutputTable = {
    .disable = clockOutputDisable,
    .enable = clockOutputEnable,
    .frequency = clockOutputFrequency,
    .ready = clockOutputReady
};

static const struct ClockClass mainClockTable = {
    .disable = mainClockDisable,
    .enable = mainClockEnable,
    .frequency = mainClockFrequency,
    .ready = mainClockReady
};

static const struct ClockClass usbClockTable = {
    .disable = usbClockDisable,
    .enable = usbClockEnable,
    .frequency = usbClockFrequency,
    .ready = usbClockReady
};

static const struct ClockClass wdtClockTable = {
    .disable = wdtClockDisable,
    .enable = wdtClockEnable,
    .frequency = wdtClockFrequency,
    .ready = wdtClockReady
};
/*----------------------------------------------------------------------------*/
static const uint32_t wdtFrequencyValues[15] = {
    600000,
    1050000,
    1400000,
    1750000,
    2100000,
    2400000,
    2700000,
    3000000,
    3250000,
    3500000,
    3750000,
    4000000,
    4200000,
    4400000,
    4600000
};
/*----------------------------------------------------------------------------*/
const struct ClockClass * const ExternalOsc = &extOscTable;
const struct ClockClass * const InternalOsc = &intOscTable;
const struct ClockClass * const WdtOsc = &wdtOscTable;
const struct ClockClass * const SystemPll = &sysPllTable;
const struct ClockClass * const UsbPll = &usbPllTable;
const struct ClockClass * const ClockOutput = &clockOutputTable;
const struct ClockClass * const MainClock = &mainClockTable;
const struct ClockClass * const UsbClock = &usbClockTable;
const struct ClockClass * const WdtClock = &wdtClockTable;
/*----------------------------------------------------------------------------*/
static uint32_t extFrequency = 0;
static uint32_t pllFrequency = 0;
static uint32_t wdtFrequency = 0;
uint32_t coreClock = INT_OSC_FREQUENCY;
/*----------------------------------------------------------------------------*/
static uint32_t calcPllFrequency(uint16_t multiplier, uint8_t divider,
    enum clockSource source)
{
  uint32_t frequency;

  switch (source)
  {
    case CLOCK_EXTERNAL:
      if (!extOscReady(ExternalOsc))
        return 0;

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

  return frequency / divider;
}
/*----------------------------------------------------------------------------*/
static enum result calcPllValues(uint16_t multiplier, uint8_t divider,
    uint32_t *result)
{
  uint8_t counter = 0;
  uint8_t msel, psel;

  if (!multiplier || !divider || divider & 1)
    return E_VALUE;

  msel = multiplier / divider - 1;
  if (msel >= 32)
    return E_VALUE;

  psel = divider >> 1;
  while (counter < 4 && psel != 1 << counter)
    counter++;
  /* Check whether actual divider value found */
  if ((psel = counter) == 4)
    return E_VALUE;

  *result = PLLCTRL_MSEL(msel) | PLLCTRL_PSEL(psel);
  return E_OK;
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
  LPC_SYSCON->SYSPLLCLKUEN = 0x01;
  LPC_SYSCON->SYSPLLCLKUEN = 0x00;
  LPC_SYSCON->SYSPLLCLKUEN = 0x01;
  /* Wait until updated */
  while (!(LPC_SYSCON->SYSPLLCLKUEN & 0x01));

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
  return intOscReady(InternalOsc) ? INT_OSC_FREQUENCY : 0;
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
  wdtFrequency = wdtFrequencyValues[index - 1] >> 1;

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
  uint32_t frequency;
  enum result res;

  res = calcPllValues(config->multiplier, config->divider, &control);
  if (res != E_OK)
    return res;

  frequency = calcPllFrequency(config->multiplier, config->divider,
      config->source);
  if (!frequency)
    return E_VALUE;

  pllFrequency = frequency;

  /* Select clock source */
  LPC_SYSCON->SYSPLLCLKSEL = config->source == CLOCK_EXTERNAL ?
      PLLCLKSEL_SYSOSC : PLLCLKSEL_IRC;

  /* Update clock source for changes to take effect */
  LPC_SYSCON->SYSPLLCLKUEN = 0x01;
  LPC_SYSCON->SYSPLLCLKUEN = 0x00;
  LPC_SYSCON->SYSPLLCLKUEN = 0x01;
  /* Wait until updated */
  while (!(LPC_SYSCON->SYSPLLCLKUEN & 0x01));

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
static void usbPllDisable(const void *clockBase __attribute__((unused)))
{
  sysPowerDisable(PWR_USBPLL);
}
/*----------------------------------------------------------------------------*/
static enum result usbPllEnable(const void *clockBase __attribute__((unused)),
    const void *configBase)
{
  const struct PllConfig * const config = configBase;
  uint32_t control; /* Control register value */
  uint32_t frequency;
  enum result res;

  res = calcPllValues(config->multiplier, config->divider, &control);
  if (res != E_OK)
    return res;

  frequency = calcPllFrequency(config->multiplier, config->divider,
      config->source);
  if (frequency != USB_FREQUENCY)
    return E_VALUE;

  /* Select clock source */
  LPC_SYSCON->USBPLLCLKSEL = config->source == CLOCK_EXTERNAL ?
      PLLCLKSEL_SYSOSC : PLLCLKSEL_IRC;

  /* Update clock source for changes to take effect */
  LPC_SYSCON->USBPLLCLKUEN = CLKUEN_ENA;
  LPC_SYSCON->USBPLLCLKUEN = 0;
  LPC_SYSCON->USBPLLCLKUEN = CLKUEN_ENA;
  /* Wait until updated */
  while (!(LPC_SYSCON->USBPLLCLKUEN & CLKUEN_ENA));

  LPC_SYSCON->USBPLLCTRL = control;
  sysPowerEnable(PWR_USBPLL);

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static uint32_t usbPllFrequency(const void *clock)
{
  return usbPllReady(clock) ? USB_FREQUENCY : 0;
}
/*----------------------------------------------------------------------------*/
static bool usbPllReady(const void *clockBase __attribute__((unused)))
{
  /* USB PLL should be locked */
  return (LPC_SYSCON->USBPLLSTAT & PLLSTAT_LOCK) != 0;
}
/*----------------------------------------------------------------------------*/
static void clockOutputDisable(const void *clockBase __attribute__((unused)))
{
  LPC_SYSCON->CLKOUTDIV = 0;
}
/*----------------------------------------------------------------------------*/
static enum result clockOutputEnable(const void *clockBase
    __attribute__((unused)), const void *configBase)
{
  const struct CommonClockConfig * const config = configBase;

  switch (config->source)
  {
    case CLOCK_INTERNAL:
      if (!intOscReady(InternalOsc))
        return E_ERROR;
      LPC_SYSCON->CLKOUTCLKSEL = CLKOUTCLKSEL_IRC;
      break;

    case CLOCK_EXTERNAL:
      if (!extOscReady(InternalOsc))
        return E_ERROR;
      LPC_SYSCON->CLKOUTCLKSEL = CLKOUTCLKSEL_SYSOSC;
      break;

    case CLOCK_WDT:
      if (!wdtOscReady(WdtOsc))
        return E_VALUE;
      LPC_SYSCON->CLKOUTCLKSEL = CLKOUTCLKSEL_WDT;
      break;

    case CLOCK_MAIN:
      LPC_SYSCON->CLKOUTCLKSEL = CLKOUTCLKSEL_MAIN_CLOCK;
      break;

    default:
      return E_ERROR;
  }

  /* Update clock source */
  LPC_SYSCON->CLKOUTUEN = CLKUEN_ENA;
  LPC_SYSCON->CLKOUTUEN = 0;
  LPC_SYSCON->CLKOUTUEN = CLKUEN_ENA;
  /* Wait until updated */
  while (!(LPC_SYSCON->CLKOUTUEN & CLKUEN_ENA));

  /* Enable clock */
  /* Enable clock */
  if (config->divider)
    LPC_SYSCON->CLKOUTDIV = config->divider;
  else
    LPC_SYSCON->CLKOUTDIV = 1;

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static uint32_t clockOutputFrequency(const void *clockBase
    __attribute__((unused)))
{
  uint32_t baseFrequency = 0;

  if (!LPC_SYSCON->CLKOUTDIV)
    return 0;

  switch (LPC_SYSCON->CLKOUTCLKSEL)
  {
    case CLKOUTCLKSEL_IRC:
      baseFrequency = intOscFrequency(InternalOsc);
      break;

    case CLKOUTCLKSEL_SYSOSC:
      baseFrequency = extOscFrequency(ExternalOsc);
      break;

    case CLKOUTCLKSEL_WDT:
      baseFrequency = wdtFrequency;
      break;

    case CLKOUTCLKSEL_MAIN_CLOCK:
      baseFrequency = mainClockFrequency(MainClock);
      break;
  }

  return baseFrequency / LPC_SYSCON->CLKOUTDIV;
}
/*----------------------------------------------------------------------------*/
static bool clockOutputReady(const void *clockBase __attribute__((unused)))
{
  return LPC_SYSCON->CLKOUTDIV != 0;
}
/*----------------------------------------------------------------------------*/
static void mainClockDisable(const void *clockBase __attribute__((unused)))
{

}
/*----------------------------------------------------------------------------*/
static enum result mainClockEnable(const void *clockBase
    __attribute__((unused)), const void *configBase)
{
  const struct CommonClockConfig * const config = configBase;

  switch (config->source)
  {
    case CLOCK_INTERNAL:
      LPC_SYSCON->MAINCLKSEL = MAINCLKSEL_IRC;
      coreClock = INT_OSC_FREQUENCY;
      break;

    case CLOCK_EXTERNAL:
      /* Check whether external oscillator is configured and ready */
      if (!extOscReady(ExternalOsc))
        return E_ERROR;
      LPC_SYSCON->MAINCLKSEL = MAINCLKSEL_PLL_INPUT;
      coreClock = extFrequency;
      break;

    case CLOCK_PLL:
      /* Check whether PLL is configured and ready */
      if (!sysPllReady(UsbPll))
        return E_ERROR;
      LPC_SYSCON->MAINCLKSEL = MAINCLKSEL_PLL_OUTPUT;
      coreClock = pllFrequency;
      break;

    case CLOCK_WDT:
      if (!wdtOscReady(WdtOsc))
        return E_ERROR;
      LPC_SYSCON->MAINCLKSEL = MAINCLKSEL_WDT;
      coreClock = wdtFrequency;
      break;

    default:
      return E_ERROR;
  }

  if (config->divider)
  {
    coreClock /= config->divider;
    LPC_SYSCON->SYSAHBCLKDIV = config->divider;
  }
  else
    LPC_SYSCON->SYSAHBCLKDIV = 1;

  /* Update Main clock source */
  LPC_SYSCON->MAINCLKUEN = CLKUEN_ENA;
  LPC_SYSCON->MAINCLKUEN = 0;
  LPC_SYSCON->MAINCLKUEN = CLKUEN_ENA;
  /* Wait until updated */
  while (!(LPC_SYSCON->MAINCLKUEN & CLKUEN_ENA));

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static uint32_t mainClockFrequency(const void *clockBase
    __attribute__((unused)))
{
  return coreClock;
}
/*----------------------------------------------------------------------------*/
static bool mainClockReady(const void *clockBase __attribute__((unused)))
{
  return true;
}
/*----------------------------------------------------------------------------*/
static void usbClockDisable(const void *clockBase __attribute__((unused)))
{
  LPC_SYSCON->USBCLKDIV = 0;
}
/*----------------------------------------------------------------------------*/
static enum result usbClockEnable(const void *clockBase __attribute__((unused)),
    const void *configBase)
{
  const struct CommonClockConfig * const config = configBase;

  if (config->divider > 1)
    return E_VALUE;

  switch (config->source)
  {
    case CLOCK_USB_PLL:
      if (!usbPllReady(UsbPll))
        return E_ERROR;
      LPC_SYSCON->USBCLKSEL = USBCLKSEL_USBPLL_OUTPUT;
      break;

    case CLOCK_MAIN:
      if (!extOscReady(ExternalOsc) || coreClock != USB_FREQUENCY)
        return E_VALUE;
      LPC_SYSCON->USBCLKSEL = USBCLKSEL_MAIN_CLOCK;
      break;

    default:
      return E_ERROR;
  }

  /* Update Main clock source */
  LPC_SYSCON->USBCLKUEN = CLKUEN_ENA;
  LPC_SYSCON->USBCLKUEN = 0;
  LPC_SYSCON->USBCLKUEN = CLKUEN_ENA;
  /* Wait until updated */
  while (!(LPC_SYSCON->USBCLKUEN & CLKUEN_ENA));

  /* Enable clock */
  LPC_SYSCON->USBCLKDIV = 1;

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static uint32_t usbClockFrequency(const void *clockBase __attribute__((unused)))
{
  switch (LPC_SYSCON->USBCLKSEL)
  {
    case USBCLKSEL_MAIN_CLOCK:
      return coreClock;

    case USBCLKSEL_USBPLL_OUTPUT:
      return usbPllFrequency(UsbPll);

    default:
      return 0;
  }
}
/*----------------------------------------------------------------------------*/
static bool usbClockReady(const void *clockBase __attribute__((unused)))
{
  return LPC_SYSCON->USBCLKDIV != 0;
}
/*----------------------------------------------------------------------------*/
static void wdtClockDisable(const void *clockBase __attribute__((unused)))
{
  LPC_SYSCON->WDTCLKDIV = 0;
}
/*----------------------------------------------------------------------------*/
static enum result wdtClockEnable(const void *clockBase __attribute__((unused)),
    const void *configBase)
{
  const struct CommonClockConfig * const config = configBase;

  switch (config->source)
  {
    case CLOCK_INTERNAL:
      if (!intOscReady(InternalOsc))
        return E_ERROR;
      LPC_SYSCON->WDTCLKSEL = WDTCLKSEL_IRC;
      break;

    case CLOCK_WDT:
      if (!wdtOscReady(WdtOsc))
        return E_VALUE;
      LPC_SYSCON->WDTCLKSEL = WDTCLKSEL_WDT;
      break;

    case CLOCK_MAIN:
      LPC_SYSCON->WDTCLKSEL = WDTCLKSEL_MAIN_CLOCK;
      break;

    default:
      return E_ERROR;
  }

  /* Update WDT clock source */
  LPC_SYSCON->WDTCLKUEN = CLKUEN_ENA;
  LPC_SYSCON->WDTCLKUEN = 0;
  LPC_SYSCON->WDTCLKUEN = CLKUEN_ENA;
  /* Wait until updated */
  while (!(LPC_SYSCON->WDTCLKUEN & CLKUEN_ENA));

  /* Enable clock */
  if (config->divider)
    LPC_SYSCON->WDTCLKDIV = config->divider;
  else
    LPC_SYSCON->WDTCLKDIV = 1;

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static uint32_t wdtClockFrequency(const void *clockBase __attribute__((unused)))
{
  uint32_t baseFrequency = 0;

  if (!LPC_SYSCON->WDTCLKDIV)
    return 0;

  switch (LPC_SYSCON->WDTCLKSEL)
  {
    case WDTCLKSEL_IRC:
      baseFrequency = intOscFrequency(InternalOsc);
      break;

    case WDTCLKSEL_MAIN_CLOCK:
      baseFrequency = mainClockFrequency(MainClock);
      break;

    case WDTCLKSEL_WDT:
      baseFrequency = wdtFrequency;
      break;
  }

  return baseFrequency / LPC_SYSCON->WDTCLKDIV;
}
/*----------------------------------------------------------------------------*/
static bool wdtClockReady(const void *clockBase __attribute__((unused)))
{
  return LPC_SYSCON->WDTCLKDIV != 0;
}
