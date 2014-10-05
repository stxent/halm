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
static bool stubReady(const void *);
/*----------------------------------------------------------------------------*/
static enum result extOscDisable(const void *);
static enum result extOscEnable(const void *, const void *);
static uint32_t extOscFrequency(const void *);
static bool extOscReady();

static enum result intOscDisable(const void *);
static enum result intOscEnable(const void *, const void *);
static uint32_t intOscFrequency(const void *);
static bool intOscReady();

static enum result sysPllDisable(const void *);
static enum result sysPllEnable(const void *, const void *);
static uint32_t sysPllFrequency(const void *);
static bool sysPllReady(const void *);

static enum result usbPllDisable(const void *);
static enum result usbPllEnable(const void *, const void *);
static uint32_t usbPllFrequency(const void *);
static bool usbPllReady(const void *);
/*----------------------------------------------------------------------------*/
static enum result mainClockDisable(const void *);
static enum result mainClockEnable(const void *, const void *);
static uint32_t mainClockFrequency(const void *);

static enum result usbClockDisable(const void *);
static enum result usbClockEnable(const void *, const void *);
static uint32_t usbClockFrequency(const void *);
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

static const struct ClockClass usbPllTable = {
    .disable = usbPllDisable,
    .enable = usbPllEnable,
    .frequency = usbPllFrequency,
    .ready = usbPllReady
};
/*----------------------------------------------------------------------------*/
static const struct ClockClass mainClockTable = {
    .disable = mainClockDisable,
    .enable = mainClockEnable,
    .frequency = mainClockFrequency,
    .ready = stubReady
};

static const struct ClockClass usbClockTable = {
    .disable = usbClockDisable,
    .enable = usbClockEnable,
    .frequency = usbClockFrequency,
    .ready = stubReady
};
/*----------------------------------------------------------------------------*/
const struct ClockClass * const ExternalOsc = &extOscTable;
const struct ClockClass * const InternalOsc = &intOscTable;
const struct ClockClass * const SystemPll = &sysPllTable;
const struct ClockClass * const UsbPll = &usbPllTable;
const struct ClockClass * const MainClock = &mainClockTable;
const struct ClockClass * const UsbClock = &usbClockTable;
/*----------------------------------------------------------------------------*/
static uint32_t extFrequency = 0;
static uint32_t pllFrequency = 0;
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
static bool stubReady(const void *clockBase __attribute__((unused)))
{
  return true;
}
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
static enum result usbPllDisable(const void *clockBase __attribute__((unused)))
{
  sysPowerDisable(PWR_USBPLL);
  return E_OK;
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
  LPC_SYSCON->USBPLLCLKUEN = 0x01;
  LPC_SYSCON->USBPLLCLKUEN = 0x00;
  LPC_SYSCON->USBPLLCLKUEN = 0x01;
  /* Wait until updated */
  while (!(LPC_SYSCON->USBPLLCLKUEN & 0x01));

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
  return LPC_SYSCON->USBPLLSTAT & PLLSTAT_LOCK;
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
      if (!sysPllReady(UsbPll))
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
static enum result usbClockDisable(const void *clockBase
    __attribute__((unused)))
{
  return E_OK;
}
/*----------------------------------------------------------------------------*/
static enum result usbClockEnable(const void *clockBase __attribute__((unused)),
    const void *configBase)
{
  const struct UsbClockConfig * const config = configBase;

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
  LPC_SYSCON->USBCLKUEN = USBCLKUEN_ENA;
  LPC_SYSCON->USBCLKUEN = 0;
  LPC_SYSCON->USBCLKUEN = USBCLKUEN_ENA;
  /* Wait until updated */
  while (!(LPC_SYSCON->USBCLKUEN & USBCLKUEN_ENA));

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static uint32_t usbClockFrequency(const void *clockBase __attribute__((unused)))
{
  switch (LPC_SYSCON->USBCLKSEL & USBCLKSEL_MASK)
  {
    case USBCLKSEL_MAIN_CLOCK:
      return coreClock;

    case USBCLKSEL_USBPLL_OUTPUT:
      return usbPllFrequency(UsbPll);

    default:
      return 0;
  }
}
