/*
 * wdt.c
 * Copyright (C) 2014 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <assert.h>
#include <halm/platform/nxp/wdt.h>
#include <halm/platform/nxp/wdt_defs.h>
#include <halm/platform/platform_defs.h>
/*----------------------------------------------------------------------------*/
static enum result wdtInit(void *, const void *);
static void wdtDeinit(void *);
static enum result wdtCallback(void *, void (*)(void *), void *);
static void wdtReload(void *);
/*----------------------------------------------------------------------------*/
static const struct WatchdogClass wdtTable = {
    .size = sizeof(struct Wdt),
    .init = wdtInit,
    .deinit = wdtDeinit,

    .callback = wdtCallback,
    .reload = wdtReload
};
/*----------------------------------------------------------------------------*/
const struct WatchdogClass * const Wdt = &wdtTable;
/*----------------------------------------------------------------------------*/
static enum result wdtInit(void *object, const void *configBase)
{
  const struct WdtConfig * const config = configBase;
  const struct WdtBaseConfig baseConfig = {
      .source = config->source
  };
  enum result res;

  /* Call base class constructor */
  if ((res = WdtBase->init(object, &baseConfig)) != E_OK)
    return res;

  const uint32_t clock = wdtGetClock(object) / 4;
  const uint32_t prescaler = config->period * (clock / 1000);

  assert(prescaler >= 1 << 8);
  assert(prescaler <= 0xFFFFFFFF >> (32 - WDT_TIMER_RESOLUTION));

  LPC_WDT->TC = prescaler;
  LPC_WDT->MOD = MOD_WDEN | MOD_WDRESET;

  wdtReload(object);

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static void wdtDeinit(void *object __attribute__((unused)))
{
  /* Watchdog timer cannot be disabled */
}
/*----------------------------------------------------------------------------*/
static enum result wdtCallback(void *object __attribute__((unused)),
    void (*callback)(void *) __attribute__((unused)),
    void *argument __attribute__((unused)))
{
  /*
   * The intent of the WDT interrupt is to allow debugging. This interrupt
   * can not be used as a regular timer interrupt.
   */
  return E_INVALID;
}
/*----------------------------------------------------------------------------*/
static void wdtReload(void *object __attribute__((unused)))
{
  const irqState state = irqSave();

  LPC_WDT->FEED = FEED_FIRST;
  LPC_WDT->FEED = FEED_SECOND;

  irqRestore(state);
}
