/*
 * wdt_base.c
 * Copyright (C) 2014 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <xcore/memory.h>
#include <halm/platform/nxp/lpc11xx/clocking.h>
#include <halm/platform/nxp/lpc11xx/system.h>
#include <halm/platform/nxp/wdt_base.h>
/*----------------------------------------------------------------------------*/
static bool setDescriptor(struct WdtBase *);
/*----------------------------------------------------------------------------*/
static enum Result wdtInit(void *, const void *);
static void wdtDeinit(void *);
/*----------------------------------------------------------------------------*/
static const struct EntityClass wdtTable = {
    .size = 0, /* Abstract class */
    .init = wdtInit,
    .deinit = wdtDeinit
};
/*----------------------------------------------------------------------------*/
const struct EntityClass * const WdtBase = &wdtTable;
static struct WdtBase *descriptor = 0;
/*----------------------------------------------------------------------------*/
static bool setDescriptor(struct WdtBase *timer)
{
  return compareExchangePointer((void **)&descriptor, 0, timer);
}
/*----------------------------------------------------------------------------*/
void WDT_ISR(void)
{
  descriptor->handler(descriptor);
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

  if (!setDescriptor(timer))
    return E_BUSY;

  timer->handler = 0;
  timer->irq = WDT_IRQ;

  sysClockEnable(CLK_WDT);

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static void wdtDeinit(void *object __attribute__((unused)))
{
  /* Watchdog timer cannot be disabled */
}
