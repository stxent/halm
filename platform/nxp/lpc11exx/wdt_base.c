/*
 * wdt_base.c
 * Copyright (C) 2014 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <assert.h>
#include <halm/platform/nxp/lpc11exx/clocking.h>
#include <halm/platform/nxp/lpc11exx/system.h>
#include <halm/platform/nxp/lpc11exx/system_defs.h>
#include <halm/platform/nxp/wdt_base.h>
#include <halm/platform/nxp/wdt_defs.h>
/*----------------------------------------------------------------------------*/
static bool setInstance(struct WdtBase *);
/*----------------------------------------------------------------------------*/
static enum Result wdtInit(void *, const void *);
/*----------------------------------------------------------------------------*/
static const struct EntityClass wdtTable = {
    .size = 0, /* Abstract class */
    .init = wdtInit,
    .deinit = 0 /* Default destructor */
};
/*----------------------------------------------------------------------------*/
const struct EntityClass * const WdtBase = &wdtTable;
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
void WWDT_ISR(void)
{
  instance->handler(instance);
}
/*----------------------------------------------------------------------------*/
uint32_t wdtGetClock(const struct WdtBase *timer __attribute__((unused)))
{
  switch (CLKSEL_WDSEL_VALUE(LPC_WDT->CLKSEL) + 1)
  {
    case WDT_CLOCK_IRC:
      return clockFrequency(InternalOsc);

    case WDT_CLOCK_WDOSC:
      return clockFrequency(WdtOsc);

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
  timer->irq = WWDT_IRQ;

  sysClockEnable(CLK_WWDT);

  const enum WdtClockSource clockSource = config->source != WDT_CLOCK_DEFAULT ?
      config->source : WDT_CLOCK_IRC;

  /* Select clock source */
  LPC_WDT->CLKSEL = CLKSEL_WDSEL(clockSource - 1) | CLKSEL_LOCK;

#ifdef CONFIG_PM
  /* Watchdog interrupt will wake the controller from low-power modes */
  LPC_SYSCON->STARTERP1 |= STARTERP1_WWDTINT;
#endif

  return E_OK;
}
