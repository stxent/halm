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
/*----------------------------------------------------------------------------*/
//TODO Move to specific header
/*------------------Main Clock Source Select Register-------------------------*/
#define MAINCLKSEL_IRC                  BIT_FIELD(0, 0)
#define MAINCLKSEL_PLL_INPUT            BIT_FIELD(1, 0)
#define MAINCLKSEL_WDT                  BIT_FIELD(2, 0)
#define MAINCLKSEL_PLL_OUTPUT           BIT_FIELD(3, 0)
/*------------------System Oscillator Control Register------------------------*/
#define SYSOSCCTRL_BYPASS               BIT(0)
#define SYSOSCCTRL_FREQRANGE            BIT(1) /* Set for 15 - 25 MHz range */
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
    .size = 0, /* Abstract class */
    .init = 0,
    .deinit = 0,

    .disable = extOscDisable,
    .enable = extOscEnable,
    .ready = extOscReady
};
/*----------------------------------------------------------------------------*/
static const struct ClockClass intOscTable = {
    .size = 0, /* Abstract class */
    .init = 0,
    .deinit = 0,

    .disable = stubDisable,
    .enable = stubEnable,
    .ready = stubReady
};
/*----------------------------------------------------------------------------*/
static const struct ClockClass mainClockTable = {
    .size = 0, /* Abstract class */
    .init = 0,
    .deinit = 0,

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
static uint32_t extOscFrequency = 0;
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

  sysPowerEnable(PWR_SYSOSC);

  if (config->bypass)
    buffer |= SYSOSCCTRL_BYPASS;
  if (config->frequency > 15e6)
    buffer |= SYSOSCCTRL_FREQRANGE;

  extOscFrequency = config->frequency;
  LPC_SYSCON->SYSOSCCTRL = buffer;

  /* There are no status register so wait 10 microseconds for startup */
  usleep(10);

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
    default:
      return E_VALUE;
  }

  /* Update Main Clock source */
  LPC_SYSCON->MAINCLKUEN = 0x01;
  LPC_SYSCON->MAINCLKUEN = 0x00;
  LPC_SYSCON->MAINCLKUEN = 0x01;

  return E_OK;
}
