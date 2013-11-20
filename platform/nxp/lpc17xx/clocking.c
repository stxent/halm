/*
 * clocking.c
 * Copyright (C) 2013 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <assert.h>
#include <platform/nxp/platform_defs.h>
#include <platform/nxp/lpc17xx/clocking.h>
#include <platform/nxp/lpc17xx/clocking_defs.h>
#include <platform/nxp/lpc17xx/system.h>
/*----------------------------------------------------------------------------*/
#define INT_OSC_FREQUENCY 4e6
/*----------------------------------------------------------------------------*/
static void stubDisable(void);
static bool stubReady(void);
/*----------------------------------------------------------------------------*/
static void extOscDisable(void);
static enum result extOscEnable(const void *);
static bool extOscReady();
static void sysPllDisable(void);
static enum result sysPllEnable(const void *);
static bool sysPllReady(void);
/*----------------------------------------------------------------------------*/
static enum result mainClockEnable(const void *);
/*----------------------------------------------------------------------------*/
static const struct ClockClass extOscTable = {
    .disable = extOscDisable,
    .enable = extOscEnable,
    .ready = extOscReady
};

static const struct ClockClass sysPllTable = {
    .disable = sysPllDisable,
    .enable = sysPllEnable,
    .ready = sysPllReady
};
/*----------------------------------------------------------------------------*/
static const struct ClockClass mainClockTable = {
    .disable = stubDisable,
    .enable = mainClockEnable,
    .ready = stubReady
};
/*----------------------------------------------------------------------------*/
const struct ClockClass *ExternalOsc = &extOscTable;
const struct ClockClass *SystemPll = &sysPllTable;
const struct ClockClass *MainClock = &mainClockTable;
/*----------------------------------------------------------------------------*/
static const uint32_t intOscFrequency = INT_OSC_FREQUENCY;
static uint32_t extOscFrequency = 0, pllFrequency = 0;
static uint8_t pllDivider = 0;
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
  //TODO
}
/*----------------------------------------------------------------------------*/
static enum result extOscEnable(const void *configPtr)
{
  const struct ExternalOscConfig * const config = configPtr;
  uint32_t buffer = 0;

//  if (config->bypass)
//    buffer |= SYSOSCCTRL_BYPASS;
  if (config->frequency > 15e6)
    buffer |= SCS_FREQRANGE;

  extOscFrequency = config->frequency;
  LPC_SC->SCS = SCS_OSCEN | buffer;

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static bool extOscReady(void)
{
  return LPC_SC->SCS & SCS_OSCSTAT ? true : false;
}
/*----------------------------------------------------------------------------*/
static void sysPllDisable(void)
{
  //TODO
}
/*----------------------------------------------------------------------------*/
static enum result sysPllEnable(const void *configPtr)
{
  const struct PllConfig * const config = configPtr;
  uint32_t frequency; /* Resulting CCO frequency */
  uint16_t msel;
  uint8_t /*nsel, */psel;

  //TODO NSEL calculation
  assert(config->multiplier && config->divider);

  msel = (config->multiplier >> 1) - 1;
  if (msel < 5 || msel >= 512)
    return E_VALUE;

  psel = config->divider - 1;
  if (psel < 2)
    return E_VALUE;

  switch (config->source)
  {
    case CLOCK_EXTERNAL:
      if (!extOscFrequency)
        return E_ERROR; /* System oscillator is not initialized */
      LPC_SC->CLKSRCSEL = CLKSRCSEL_MAIN;
      frequency = extOscFrequency;
      break;
    case CLOCK_INTERNAL:
      LPC_SC->CLKSRCSEL = CLKSRCSEL_IRC;
      frequency = intOscFrequency;
      break;
    default:
      return E_ERROR;
  }

  /* Check CCO range */
  frequency = frequency * config->multiplier;
  if (frequency < 275e6 || frequency > 550e6)
    return E_ERROR;
  pllFrequency = frequency / config->divider;
  pllDivider = psel;

  /* Update PLL clock source */
  LPC_SC->PLL0CFG = PLL0CFG_MSEL(msel) | PLL0CFG_NSEL(0);
  LPC_SC->PLL0FEED = 0xAA;
  LPC_SC->PLL0FEED = 0x55;

  /* Enable PLL */
  LPC_SC->PLL0CON = PLL0CON_ENABLE;
  LPC_SC->PLL0FEED  = 0xAA;
  LPC_SC->PLL0FEED  = 0x55;

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static bool sysPllReady(void)
{
  return LPC_SC->PLL0STAT & PLL0STAT_LOCK ? true : false;
}
/*----------------------------------------------------------------------------*/
static enum result mainClockEnable(const void *configPtr)
{
  const struct MainClockConfig * const config = configPtr;

  switch (config->source)
  {
    case CLOCK_INTERNAL:
      LPC_SC->CLKSRCSEL = CLKSRCSEL_IRC;
      sysCoreClock = intOscFrequency;
    case CLOCK_EXTERNAL:
      /* Check whether external oscillator is configured */
      if (!extOscFrequency)
        return E_ERROR;
      LPC_SC->CLKSRCSEL = CLKSRCSEL_MAIN;
      sysCoreClock = extOscFrequency;
      break;
    case CLOCK_PLL:
      /* Check whether PLL is configured */
      if (!pllFrequency)
        return E_ERROR;

      /* Configure system clock divider */
      LPC_SC->CCLKCFG = pllDivider;

      /* Connect PLL */
      LPC_SC->PLL0CON |= PLL0CON_CONNECT;
      LPC_SC->PLL0FEED = 0xAA;
      LPC_SC->PLL0FEED = 0x55;

      /* Wait for enable and connect */
      while (!(LPC_SC->PLL0STAT & (PLL0STAT_ENABLED | PLL0STAT_CONNECTED)));

      sysCoreClock = pllFrequency;
      break;
    case CLOCK_RTC:
      LPC_SC->CLKSRCSEL = CLKSRCSEL_RTC;
      break;
    default:
      return E_ERROR;
  }

  return E_OK;
}
