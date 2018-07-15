/*
 * wdt_base.c
 * Copyright (C) 2014 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <halm/platform/nxp/lpc11xx/clocking.h>
#include <halm/platform/nxp/lpc11xx/system.h>
#include <halm/platform/nxp/wdt_base.h>
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
void WDT_ISR(void)
{
  instance->handler(instance);
}
/*----------------------------------------------------------------------------*/
uint32_t wdtGetClock(const struct WdtBase *timer __attribute__((unused)))
{
  return clockFrequency(WdtClock);
}
/*----------------------------------------------------------------------------*/
static enum Result wdtInit(void *object, const void *configBase
    __attribute__((unused)))
{
  struct WdtBase * const timer = object;

  if (setInstance(timer))
  {
    timer->handler = 0;
    timer->irq = WDT_IRQ;
    sysClockEnable(CLK_WDT);
    return E_OK;
  }
  else
    return E_BUSY;
}
