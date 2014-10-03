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
#define INT_OSC_FREQUENCY 12000000
/*----------------------------------------------------------------------------*/
static enum result extOscDisable(const void *);
static enum result extOscEnable(const void *, const void *);
static uint32_t extOscFrequency(const void *);
static bool extOscReady(const void *);

static enum result intOscDisable(const void *);
static enum result intOscEnable(const void *, const void *);
static uint32_t intOscFrequency(const void *);
static bool intOscReady(const void *);

static enum result sysPllDisable(const void *);
static enum result sysPllEnable(const void *, const void *);
static uint32_t sysPllFrequency(const void *);
static bool sysPllReady(const void *);
/*----------------------------------------------------------------------------*/
static enum result mainClockDisable(const void *);
static enum result mainClockEnable(const void *, const void *);
static uint32_t mainClockFrequency(const void *);
static bool mainClockReady(const void *);
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

static const struct ClockClass sysPllTable = {
    .disable = sysPllDisable,
    .enable = sysPllEnable,
    .frequency = sysPllFrequency,
    .ready = sysPllReady
};
/*----------------------------------------------------------------------------*/
static const struct ClockClass mainClockTable = {
    .disable = mainClockDisable,
    .enable = mainClockEnable,
    .frequency = mainClockFrequency,
    .ready = mainClockReady
};
/*----------------------------------------------------------------------------*/
const struct ClockClass * const ExternalOsc = &extOscTable;
const struct ClockClass * const InternalOsc = &intOscTable;
const struct ClockClass * const SystemPll = &sysPllTable;
const struct ClockClass * const MainClock = &mainClockTable;
/*----------------------------------------------------------------------------*/
static uint32_t extFrequency = 0;
static uint32_t pllFrequency = 0;
uint32_t coreClock = INT_OSC_FREQUENCY;
/*----------------------------------------------------------------------------*/
static enum result extOscDisable(const void *clockBase __attribute__((unused)))
{
  if ((LPC_SYSCON->MAINCLKSEL & MAINCLKSEL_MASK) != MAINCLKSEL_PLL_INPUT)
  {
    sysPowerDisable(PWR_SYSOSC);
    return E_OK;
  }
  else
    return E_ERROR;
}
/*----------------------------------------------------------------------------*/
static enum result extOscEnable(const void *clockBase __attribute__((unused)),
    const void *configBase)
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
  return extFrequency && sysPowerStatus(PWR_SYSOSC);
}
/*----------------------------------------------------------------------------*/
static enum result intOscDisable(const void *clockBase __attribute__((unused)))
{
  if ((LPC_SYSCON->MAINCLKSEL & MAINCLKSEL_MASK) != MAINCLKSEL_IRC)
  {
    sysPowerDisable(PWR_IRCOUT);
    sysPowerDisable(PWR_IRC);
    return E_OK;
  }
  else
    return E_ERROR;
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
  return INT_OSC_FREQUENCY;
}
/*----------------------------------------------------------------------------*/
static bool intOscReady(const void *clockBase __attribute__((unused)))
{
  return sysPowerStatus(PWR_IRC) && sysPowerStatus(PWR_IRCOUT);
}
/*----------------------------------------------------------------------------*/
static enum result sysPllDisable(const void *clockBase __attribute__((unused)))
{
  if ((LPC_SYSCON->MAINCLKSEL & MAINCLKSEL_MASK) != MAINCLKSEL_PLL_OUTPUT)
  {
    sysPowerDisable(PWR_SYSPLL);
    return E_OK;
  }
  else
    return E_ERROR;
}
/*----------------------------------------------------------------------------*/
static enum result sysPllEnable(const void *clockBase __attribute__((unused)),
    const void *configBase)
{
  const struct PllConfig * const config = configBase;
  uint32_t frequency; /* Resulting CCO frequency */
  uint8_t msel, psel, counter = 0;

  if (!config->multiplier || !config->divider || config->divider & 1)
    return E_VALUE;

  msel = config->multiplier / config->divider - 1;
  if (msel >= 32)
    return E_VALUE;

  psel = config->divider >> 1;
  while (counter < 4 && psel != 1 << counter)
    counter++;
  /* Check whether actual divider value found */
  if ((psel = counter) == 4)
    return E_VALUE;

  switch (config->source)
  {
    case CLOCK_EXTERNAL:
      if (!extOscReady(ExternalOsc))
        return E_ERROR;
      LPC_SYSCON->SYSPLLCLKSEL = PLLCLKSEL_SYSOSC;
      frequency = extFrequency;
      break;

    case CLOCK_INTERNAL:
      LPC_SYSCON->SYSPLLCLKSEL = PLLCLKSEL_IRC;
      frequency = INT_OSC_FREQUENCY;
      break;

    default:
      return E_ERROR;
  }

  /* Check CCO range */
  frequency = frequency * config->multiplier;
  if (frequency < 156000000 || frequency > 320000000)
    return E_ERROR;
  pllFrequency = frequency / config->divider;

  /* Update PLL clock source */
  LPC_SYSCON->SYSPLLCLKUEN = 0x01;
  LPC_SYSCON->SYSPLLCLKUEN = 0x00;
  LPC_SYSCON->SYSPLLCLKUEN = 0x01;
  /* Wait until updated */
  while (!(LPC_SYSCON->SYSPLLCLKUEN & 0x01));

  LPC_SYSCON->SYSPLLCTRL = PLLCTRL_MSEL(msel) | PLLCTRL_PSEL(psel);
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
static enum result mainClockDisable(const void *clockBase
    __attribute__((unused)))
{
  return E_ERROR;
}
/*----------------------------------------------------------------------------*/
static enum result mainClockEnable(const void *clockBase
    __attribute__((unused)), const void *configBase)
{
  const struct MainClockConfig * const config = configBase;

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
      if (!sysPllReady(SystemPll))
        return E_ERROR;
      LPC_SYSCON->MAINCLKSEL = MAINCLKSEL_PLL_OUTPUT;
      coreClock = pllFrequency;
      break;

    case CLOCK_WDT:
      //TODO
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
  LPC_SYSCON->MAINCLKUEN = MAINCLKUEN_ENA;
  LPC_SYSCON->MAINCLKUEN = 0;
  LPC_SYSCON->MAINCLKUEN = MAINCLKUEN_ENA;
  /* Wait until updated */
  while (!(LPC_SYSCON->MAINCLKUEN & MAINCLKUEN_ENA));

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
