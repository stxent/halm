/*
 * clocking.c
 * Copyright (C) 2013 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <assert.h>
#include <delay.h>
#include <platform/nxp/platform_defs.h>
#include <platform/nxp/lpc13xx/clocking.h>
#include <platform/nxp/lpc13xx/clocking_defs.h>
#include <platform/nxp/lpc13xx/system.h>
/*----------------------------------------------------------------------------*/
#define INT_OSC_FREQUENCY 12000000
/*----------------------------------------------------------------------------*/
static void stubDisable(void);
static bool stubReady(void);
/*----------------------------------------------------------------------------*/
static void extOscDisable(void);
static enum result extOscEnable(const void *);
static bool extOscReady();
static void intOscDisable(void);
static enum result intOscEnable(const void *);
static bool intOscReady();
static void sysPllDisable(void);
static enum result sysPllEnable(const void *);
static bool sysPllReady(void);
static void usbPllDisable(void);
static enum result usbPllEnable(const void *);
static bool usbPllReady(void);
/*----------------------------------------------------------------------------*/
static enum result mainClockEnable(const void *);
static enum result usbClockEnable(const void *);
/*----------------------------------------------------------------------------*/
static const struct ClockClass extOscTable = {
    .disable = extOscDisable,
    .enable = extOscEnable,
    .ready = extOscReady
};

static const struct ClockClass intOscTable = {
    .disable = intOscDisable,
    .enable = intOscEnable,
    .ready = intOscReady
};

static const struct ClockClass sysPllTable = {
    .disable = sysPllDisable,
    .enable = sysPllEnable,
    .ready = sysPllReady
};

static const struct ClockClass usbPllTable = {
    .disable = usbPllDisable,
    .enable = usbPllEnable,
    .ready = usbPllReady
};
/*----------------------------------------------------------------------------*/
static const struct ClockClass mainClockTable = {
    .disable = stubDisable,
    .enable = mainClockEnable,
    .ready = stubReady
};
/*----------------------------------------------------------------------------*/
static const struct ClockClass usbClockTable = {
    .disable = stubDisable,
    .enable = usbClockEnable,
    .ready = stubReady
};
/*----------------------------------------------------------------------------*/
const struct ClockClass *ExternalOsc = &extOscTable;
const struct ClockClass *InternalOsc = &intOscTable;
const struct ClockClass *SystemPll = &sysPllTable;
const struct ClockClass *UsbPll = &usbPllTable;
const struct ClockClass *MainClock = &mainClockTable;
const struct ClockClass *UsbClock = &usbClockTable;
/*----------------------------------------------------------------------------*/
static const uint32_t intOscFrequency = INT_OSC_FREQUENCY;
static uint32_t extOscFrequency = 0, pllFrequency = 0;
uint32_t sysCoreClock = INT_OSC_FREQUENCY;
/*----------------------------------------------------------------------------*/
static void stubDisable(void)
{

}
/*----------------------------------------------------------------------------*/
static bool stubReady(void)
{
  return true;
}
/*----------------------------------------------------------------------------*/
static void extOscDisable(void)
{
  sysPowerDisable(PWR_SYSOSC);
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

  extOscFrequency = config->frequency;
  LPC_SYSCON->SYSOSCCTRL = buffer;

  sysPowerEnable(PWR_SYSOSC);

  /* There is no status register so wait 10 microseconds for startup */
  udelay(10);

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
static bool extOscReady(void)
{
  //TODO
  return true;
}
/*----------------------------------------------------------------------------*/
static void intOscDisable(void)
{
  sysPowerDisable(PWR_IRCOUT);
  sysPowerDisable(PWR_IRC);
}
/*----------------------------------------------------------------------------*/
static enum result intOscEnable(const void *configPtr __attribute__((unused)))
{
  sysPowerEnable(PWR_IRC);
  sysPowerEnable(PWR_IRCOUT);

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static bool intOscReady(void)
{
  //TODO
  return true;
}
/*----------------------------------------------------------------------------*/
static void sysPllDisable(void)
{
  sysPowerDisable(PWR_SYSPLL);
}
/*----------------------------------------------------------------------------*/
static enum result sysPllEnable(const void *configPtr)
{
  const struct PllConfig * const config = configPtr;
  uint32_t frequency; /* Resulting CCO frequency */
  uint8_t msel, psel, counter = 0;

  assert(config->multiplier && config->divider && !(config->divider & 1));

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
      if (!extOscFrequency)
        return E_ERROR; /* System oscillator is not initialized */
      LPC_SYSCON->SYSPLLCLKSEL = PLLCLKSEL_SYSOSC;
      frequency = extOscFrequency;
      break;
    case CLOCK_INTERNAL:
      LPC_SYSCON->SYSPLLCLKSEL = PLLCLKSEL_IRC;
      frequency = intOscFrequency;
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
static bool sysPllReady(void)
{
  return LPC_SYSCON->SYSPLLSTAT & PLLSTAT_LOCK ? true : false;
}
/*----------------------------------------------------------------------------*/
static void usbPllDisable(void)
{
  sysPowerDisable(PWR_USBPLL);
}
/*----------------------------------------------------------------------------*/
static enum result usbPllEnable(const void *configPtr)
{
  const struct PllConfig * const config = configPtr;
  uint32_t frequency; /* Resulting CCO frequency */
  uint8_t msel, psel, counter = 0;

  assert(config->multiplier && config->divider && config->divider ^ 1);

  if (config->source != CLOCK_EXTERNAL || !extOscFrequency)
    return E_ERROR;

  //TODO LPC13xx clock: thoroughly check the selection of input variables style
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
  frequency = extOscFrequency * config->multiplier;
  if (frequency < 156000000 || frequency > 320000000)
    return E_ERROR;
  frequency /= config->divider;
  if (frequency != 48000000)
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
static bool usbPllReady(void)
{
  return LPC_SYSCON->USBPLLSTAT & PLLSTAT_LOCK ? true : false;
}
/*----------------------------------------------------------------------------*/
static enum result mainClockEnable(const void *configPtr)
{
  const struct MainClockConfig * const config = configPtr;

  //TODO In LPC13xx clocking: add more configuration checks
  switch (config->source)
  {
    case CLOCK_INTERNAL:
      LPC_SYSCON->MAINCLKSEL = MAINCLKSEL_IRC;
      sysCoreClock = intOscFrequency;
    case CLOCK_EXTERNAL:
      /* Check whether external oscillator is configured */
      if (!extOscFrequency)
        return E_ERROR;
      LPC_SYSCON->MAINCLKSEL = MAINCLKSEL_PLL_INPUT;
      sysCoreClock = extOscFrequency;
      break;
    case CLOCK_PLL:
      /* Check whether PLL is configured */
      if (!pllFrequency)
        return E_ERROR;
      LPC_SYSCON->MAINCLKSEL = MAINCLKSEL_PLL_OUTPUT;
      sysCoreClock = pllFrequency;
      break;
    case CLOCK_WDT:
      //TODO
      break;
    default:
      return E_ERROR;
  }

  /* Update Main clock source */
  LPC_SYSCON->MAINCLKUEN = 0x01;
  LPC_SYSCON->MAINCLKUEN = 0x00;
  LPC_SYSCON->MAINCLKUEN = 0x01;
  /* Wait until updated */
  while (!(LPC_SYSCON->MAINCLKUEN & 0x01));

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static enum result usbClockEnable(const void *configPtr)
{
  const struct MainClockConfig * const config = configPtr; //FIXME

//  if (!extOscFrequency)
//    return E_ERROR;

  //TODO Add more checks
  switch (config->source)
  {
    case CLOCK_USB_PLL:
      LPC_SYSCON->USBCLKSEL = USBCLKSEL_USBPLL_OUTPUT;
      break;
    case CLOCK_MAIN:
      LPC_SYSCON->USBCLKSEL = USBCLKSEL_MAIN_CLOCK;
      break;
    default:
      return E_ERROR;
  }

  /* Update Main clock source */
  LPC_SYSCON->USBCLKUEN = 0x01;
  LPC_SYSCON->USBCLKUEN = 0x00;
  LPC_SYSCON->USBCLKUEN = 0x01;
  /* Wait until updated */
  while (!(LPC_SYSCON->USBCLKUEN & 0x01));

  return E_OK;
}
