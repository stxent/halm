/*
 * wdt_base.c
 * Copyright (C) 2014 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <assert.h>
#include <xcore/memory.h>
#include <halm/platform/nxp/lpc17xx/clocking.h>
#include <halm/platform/nxp/lpc17xx/system.h>
#include <halm/platform/nxp/wdt_base.h>
#include <halm/platform/nxp/wdt_defs.h>
/*----------------------------------------------------------------------------*/
#define DEFAULT_DIV CLK_DIV1
/*----------------------------------------------------------------------------*/
static bool setDescriptor(struct WdtBase *);
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
static enum result wdtInit(void *object, const void *configBase)
{
  const struct WdtBaseConfig * const config = configBase;
  struct WdtBase * const timer = object;

  assert(config->source < WDT_CLOCK_END);

  if (!setDescriptor(timer))
    return E_BUSY;

  timer->handler = 0;
  timer->irq = WDT_IRQ;

  sysClockControl(CLK_WDT, DEFAULT_DIV);

  const enum wdtClockSource clockSource = config->source != WDT_CLOCK_DEFAULT ?
      config->source : WDT_CLOCK_IRC;

  LPC_WDT->CLKSEL = CLKSEL_WDSEL(clockSource - 1) | CLKSEL_LOCK;

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static void wdtDeinit(void *object __attribute__((unused)))
{
  /* Watchdog timer cannot be disabled */
}
