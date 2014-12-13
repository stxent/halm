/*
 * wwdt_base.c
 * Copyright (C) 2014 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <memory.h>
#include <platform/nxp/wwdt_base.h>
#include <platform/nxp/wwdt_defs.h>
#include <platform/nxp/lpc11exx/clocking.h>
#include <platform/nxp/lpc11exx/system.h>
/*----------------------------------------------------------------------------*/
static enum result setDescriptor(struct WwdtBase *);
/*----------------------------------------------------------------------------*/
static enum result wdtInit(void *, const void *);
static void wdtDeinit(void *);
/*----------------------------------------------------------------------------*/
static const struct EntityClass wdtTable = {
    .size = 0, /* Abstract class */
    .init = wdtInit,
    .deinit = wdtDeinit
};
/*----------------------------------------------------------------------------*/
const struct EntityClass * const WwdtBase = &wdtTable;
static struct WwdtBase *descriptor = 0;
/*----------------------------------------------------------------------------*/
static enum result setDescriptor(struct WwdtBase *timer)
{
  return compareExchangePointer((void **)&descriptor, 0, timer) ? E_OK : E_BUSY;
}
/*----------------------------------------------------------------------------*/
void WWDT_ISR(void)
{
  descriptor->handler(descriptor);
}
/*----------------------------------------------------------------------------*/
uint32_t wwdtGetClock(const struct WwdtBase *timer __attribute__((unused)))
{
  return clockFrequency(WdtOsc);
}
/*----------------------------------------------------------------------------*/
static enum result wdtInit(void *object, const void *configBase
    __attribute__((unused)))
{
  struct WwdtBase * const timer = object;
  const enum result res = setDescriptor(timer);

  if (res != E_OK)
    return res;

  timer->handler = 0;
  timer->irq = WWDT_IRQ;

  sysClockEnable(CLK_WWDT);

  /* Select clock source */
  LPC_WDT->CLKSEL = CLKSEL_WDOSC;

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static void wdtDeinit(void *object __attribute__((unused)))
{
  /* Watchdog timer cannot be disabled */
}
