/*
 * wdt_base.c
 * Copyright (C) 2014 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <halm/platform/lpc/lpc17xx/clocking.h>
#include <halm/platform/lpc/lpc17xx/system.h>
#include <halm/platform/lpc/wdt_base.h>
#include <halm/platform/lpc/wdt_defs.h>
#include <assert.h>
/*----------------------------------------------------------------------------*/
#define DEFAULT_DIV CLK_DIV1
/*----------------------------------------------------------------------------*/
static bool setInstance(struct WdtBase *);

static enum Result wdtInit(void *, const void *);
/*----------------------------------------------------------------------------*/
const struct EntityClass * const WdtBase = &(const struct EntityClass){
    .size = 0, /* Abstract class */
    .init = wdtInit,
    .deinit = 0 /* Default destructor */
};
/*----------------------------------------------------------------------------*/
static struct WdtBase *instance = 0;
/*----------------------------------------------------------------------------*/
static bool setInstance(struct WdtBase *object)
{
  if (!instance)
  {
    instance = object;
    return true;
  }
  else
    return false;
}
/*----------------------------------------------------------------------------*/
uint32_t wdtGetClock(const struct WdtBase *timer __attribute__((unused)))
{
  switch (CLKSEL_WDSEL_VALUE(LPC_WDT->CLKSEL) + 1)
  {
    case WDT_CLOCK_IRC:
      return clockFrequency(InternalOsc);

    case WDT_CLOCK_PCLK:
      return clockFrequency(MainClock) / sysClockDivToValue(DEFAULT_DIV);

    case WDT_CLOCK_RTC:
      return clockFrequency(RtcOsc);

    default:
      return 0;
  }
}
/*----------------------------------------------------------------------------*/
static enum Result wdtInit(void *object, const void *configBase)
{
  const struct WdtBaseConfig * const config = configBase;
  struct WdtBase * const timer = object;

  assert(config->source < WDT_CLOCK_END);

  if (!setInstance(timer))
    return E_BUSY;

  timer->handler = 0;
  timer->irq = WDT_IRQ;

  sysClockControl(CLK_WDT, DEFAULT_DIV);

  const enum WdtClockSource clockSource = config->source != WDT_CLOCK_DEFAULT ?
      config->source : WDT_CLOCK_IRC;
  LPC_WDT->CLKSEL = CLKSEL_WDSEL(clockSource - 1) | CLKSEL_LOCK;

  return E_OK;
}
