/*
 * clocking.c
 * Copyright (C) 2013 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <delay.h>
#include <platform/nxp/platform_defs.h>
#include <platform/nxp/lpc13xx/clocking.h>
#include <platform/nxp/lpc13xx/clocking_defs.h>
#include <platform/nxp/lpc13xx/system.h>
/*----------------------------------------------------------------------------*/
#define INT_OSC_FREQUENCY 12000000
#define USB_FREQUENCY     48000000
/*----------------------------------------------------------------------------*/
static enum result stubDisable(void);
static bool stubReady(void);
/*----------------------------------------------------------------------------*/
static enum result extOscDisable(void);
static enum result extOscEnable(const void *);
static uint32_t extOscFrequency(void);
static bool extOscReady();

static enum result intOscDisable(void);
static enum result intOscEnable(const void *);
static uint32_t intOscFrequency(void);
static bool intOscReady();

static enum result sysPllDisable(void);
static enum result sysPllEnable(const void *);
static uint32_t sysPllFrequency(void);
static bool sysPllReady(void);

static enum result usbPllDisable(void);
static enum result usbPllEnable(const void *);
static uint32_t usbPllFrequency(void);
static bool usbPllReady(void);
/*----------------------------------------------------------------------------*/
static enum result mainClockEnable(const void *);
static uint32_t mainClockFrequency(void);

static enum result usbClockDisable();
static enum result usbClockEnable(const void *);
static uint32_t usbClockFrequency(void);
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
    .disable = stubDisable,
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
static enum result stubDisable(void)
{
  return E_ERROR;
}
/*----------------------------------------------------------------------------*/
static bool stubReady(void)
{
  return true;
}
/*----------------------------------------------------------------------------*/
static enum result extOscDisable(void)
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
static enum result extOscEnable(const void *configPtr)
{
  const struct ExternalOscConfig * const config = configPtr;
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
static uint32_t extOscFrequency(void)
{
  return extFrequency;
}
/*----------------------------------------------------------------------------*/
static bool extOscReady(void)
{
  return extFrequency && sysPowerStatus(PWR_SYSOSC);
}
/*----------------------------------------------------------------------------*/
static enum result intOscDisable(void)
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
static enum result intOscEnable(const void *configPtr __attribute__((unused)))
{
  sysPowerEnable(PWR_IRC);
  sysPowerEnable(PWR_IRCOUT);

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static uint32_t intOscFrequency(void)
{
  return INT_OSC_FREQUENCY;
}
/*----------------------------------------------------------------------------*/
static bool intOscReady(void)
{
  return sysPowerStatus(PWR_IRC) && sysPowerStatus(PWR_IRCOUT);
}
/*----------------------------------------------------------------------------*/
static enum result sysPllDisable(void)
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
static enum result sysPllEnable(const void *configPtr)
{
  const struct PllConfig * const config = configPtr;
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
      if (!extOscReady())
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
static uint32_t sysPllFrequency(void)
{
  return pllFrequency;
}
/*----------------------------------------------------------------------------*/
static bool sysPllReady(void)
{
  return pllFrequency && (LPC_SYSCON->SYSPLLSTAT & PLLSTAT_LOCK ? true : false);
}
/*----------------------------------------------------------------------------*/
static enum result usbPllDisable(void)
{
  sysPowerDisable(PWR_USBPLL);
  return E_OK;
}
/*----------------------------------------------------------------------------*/
static enum result usbPllEnable(const void *configPtr)
{
  const struct PllConfig * const config = configPtr;
  uint32_t frequency; /* Resulting CCO frequency */
  uint8_t msel, psel, counter = 0;

  if (!config->multiplier || !config->divider || config->divider & 1)
    return E_VALUE;

  if (config->source != CLOCK_EXTERNAL || !extFrequency)
    return E_ERROR;

  msel = config->multiplier / config->divider - 1;
  if (msel >= 32)
    return E_VALUE;

  psel = config->divider >> 1;
  while (counter < 4 && psel != 1 << counter)
    counter++;
  /* Check whether actual divider value found */
  if ((psel = counter) == 4)
    return E_VALUE;

  /* Check CCO range */
  frequency = extFrequency * config->multiplier;
  if (frequency < 156000000 || frequency > 320000000)
    return E_ERROR;
  frequency /= config->divider;
  if (frequency != USB_FREQUENCY)
    return E_ERROR;

  LPC_SYSCON->USBPLLCLKSEL = PLLCLKSEL_SYSOSC;

  /* Update USB PLL clock source */
  LPC_SYSCON->USBPLLCLKUEN = 0x01;
  LPC_SYSCON->USBPLLCLKUEN = 0x00;
  LPC_SYSCON->USBPLLCLKUEN = 0x01;
  /* Wait until updated */
  while (!(LPC_SYSCON->USBPLLCLKUEN & 0x01));

  LPC_SYSCON->USBPLLCTRL = PLLCTRL_MSEL(msel) | PLLCTRL_PSEL(psel);
  sysPowerEnable(PWR_USBPLL);

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static uint32_t usbPllFrequency(void)
{
  return usbPllReady() ? USB_FREQUENCY : 0;
}
/*----------------------------------------------------------------------------*/
static bool usbPllReady(void)
{
  /* System oscillator should be selected as the USB PLL source */
  if ((LPC_SYSCON->USBPLLCLKSEL & USBCLKSEL_MASK) != PLLCLKSEL_SYSOSC)
    return false;

  /* USB PLL should be locked */
  if (!(LPC_SYSCON->USBPLLSTAT & PLLSTAT_LOCK))
    return false;

  return true;
}
/*----------------------------------------------------------------------------*/
static enum result mainClockEnable(const void *configPtr)
{
  const struct MainClockConfig * const config = configPtr;

  switch (config->source)
  {
    case CLOCK_INTERNAL:
      LPC_SYSCON->MAINCLKSEL = MAINCLKSEL_IRC;
      coreClock = INT_OSC_FREQUENCY;

    case CLOCK_EXTERNAL:
      /* Check whether external oscillator is configured and ready */
      if (!extOscReady())
        return E_ERROR;
      LPC_SYSCON->MAINCLKSEL = MAINCLKSEL_PLL_INPUT;
      coreClock = extFrequency;
      break;

    case CLOCK_PLL:
      /* Check whether PLL is configured and ready */
      if (!sysPllReady())
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

  /* Update Main clock source */
  LPC_SYSCON->MAINCLKUEN = MAINCLKUEN_ENA;
  LPC_SYSCON->MAINCLKUEN = 0;
  LPC_SYSCON->MAINCLKUEN = MAINCLKUEN_ENA;
  /* Wait until updated */
  while (!(LPC_SYSCON->MAINCLKUEN & MAINCLKUEN_ENA));

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static uint32_t mainClockFrequency(void)
{
  return coreClock;
}
/*----------------------------------------------------------------------------*/
static enum result usbClockDisable(void)
{
  return E_OK;
}
/*----------------------------------------------------------------------------*/
static enum result usbClockEnable(const void *configPtr)
{
  const struct UsbClockConfig * const config = configPtr;

  switch (config->source)
  {
    case CLOCK_USB_PLL:
      if (!usbPllReady())
        return E_ERROR;
      LPC_SYSCON->USBCLKSEL = USBCLKSEL_USBPLL_OUTPUT;
      break;

    case CLOCK_MAIN:
      if (!extOscReady() || coreClock != USB_FREQUENCY)
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
static uint32_t usbClockFrequency(void)
{
  switch (LPC_SYSCON->USBCLKSEL & USBCLKSEL_MASK)
  {
    case USBCLKSEL_MAIN_CLOCK:
      return coreClock;

    case USBCLKSEL_USBPLL_OUTPUT:
      return usbPllFrequency();

    default:
      return 0;
  }
}
