/*
 * clocking.c
 * Copyright (C) 2013 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include "macro.h"
#include "platform/nxp/device_defs.h"
#include "platform/nxp/system.h"
#include "platform/nxp/lpc13xx/power.h"
#include "platform/nxp/lpc13xx/clocking.h"
#include "platform/nxp/lpc13xx/clocking_defs.h"
/*----------------------------------------------------------------------------*/
void stubDisable(void);
enum result stubEnable(const void *);
bool stubReady(void);
/*----------------------------------------------------------------------------*/
void extOscDisable(void);
enum result extOscEnable(const void *);
bool extOscReady(void);
/*----------------------------------------------------------------------------*/
//void intOscDisable(void);
//enum result intOscEnable(const void *);
//bool intOscReady(void);
/*----------------------------------------------------------------------------*/
enum result mainClockEnable(const void *);
/*----------------------------------------------------------------------------*/
static const struct ClockClass extOscTable = {
    .disable = extOscDisable,
    .enable = extOscEnable,
    .ready = extOscReady
};
/*----------------------------------------------------------------------------*/
static const struct ClockClass intOscTable = {
    .disable = stubDisable,
    .enable = stubEnable,
    .ready = stubReady
};
/*----------------------------------------------------------------------------*/
static const struct ClockClass mainClockTable = {
    .disable = stubDisable,
    .enable = mainClockEnable,
    .ready = stubReady
};
///*----------------------------------------------------------------------------*/
const struct ClockClass *ExternalOsc = &extOscTable;
const struct ClockClass *InternalOsc = &intOscTable;
const struct ClockClass *MainClock = &mainClockTable;
//const struct ClockClass *SystemPll = &systemPllTable;
/*----------------------------------------------------------------------------*/
static const uint32_t intOscFrequency = 12000000;
static uint32_t extOscFrequency = 0, pllFrequency = 0;
/*----------------------------------------------------------------------------*/
//TODO Move declaration from system.h to other file
uint32_t sysCoreClock = 12000000; //FIXME
/*----------------------------------------------------------------------------*/
void stubDisable(void)
{

}
/*----------------------------------------------------------------------------*/
enum result stubEnable(const void *configPtr __attribute__((unused)))
{
  return E_ERROR;
}
/*----------------------------------------------------------------------------*/
bool stubReady(void)
{
  return false;
}
/*----------------------------------------------------------------------------*/
void extOscDisable(void)
{
  sysPowerDisable(PWR_SYSOSC);
}
/*----------------------------------------------------------------------------*/
enum result extOscEnable(const void *configPtr)
{
  const struct ExternalOscConfig * const config = configPtr;
  uint32_t buffer = 0;

  if (config->bypass)
    buffer |= SYSOSCCTRL_BYPASS;
  if (config->frequency > 15e6)
    buffer |= SYSOSCCTRL_FREQRANGE;

  extOscFrequency = config->frequency;
  LPC_SYSCON->SYSOSCCTRL = buffer;

  sysPowerEnable(PWR_SYSOSC);

  /* There is no status register so wait 10 microseconds for startup */
  usleep(10);

  LPC_SYSCON->SYSPLLCLKSEL = SYSPLLCLKSEL_SYSOSC;
  /* Update PLL clock source */
  LPC_SYSCON->SYSPLLCLKUEN = 0x01;
  LPC_SYSCON->SYSPLLCLKUEN = 0x00;
  LPC_SYSCON->SYSPLLCLKUEN = 0x01;
  /* Wait until updated */
  while (!(LPC_SYSCON->SYSPLLCLKUEN & 0x01));

  return E_OK;
}
/*----------------------------------------------------------------------------*/
bool extOscReady(void)
{
  return true;
}
/*----------------------------------------------------------------------------*/
enum result mainClockEnable(const void *configPtr)
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
    default:
      return E_VALUE;
  }

  /* Update Main clock source */
  LPC_SYSCON->MAINCLKUEN = 0x01;
  LPC_SYSCON->MAINCLKUEN = 0x00;
  LPC_SYSCON->MAINCLKUEN = 0x01;
  /* Wait until updated */
  while (!(LPC_SYSCON->MAINCLKUEN & 0x01));

  return E_OK;
}
