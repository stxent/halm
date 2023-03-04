/*
 * wdt.c
 * Copyright (C) 2014 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/platform/lpc/wdt.h>
#include <halm/platform/lpc/wdt_defs.h>
#include <halm/platform/platform_defs.h>
#include <assert.h>
/*----------------------------------------------------------------------------*/
static void reloadCounter(void);
/*----------------------------------------------------------------------------*/
static enum Result wdtInit(void *, const void *);
static bool wdtFired(const void *);
static void wdtReload(void *);
/*----------------------------------------------------------------------------*/
const struct WatchdogClass * const Wdt = &(const struct WatchdogClass){
    .size = sizeof(struct Wdt),
    .init = wdtInit,
    .deinit = 0, /* Default destructor */

    .fired = wdtFired,
    .setCallback = 0,
    .reload = wdtReload
};
/*----------------------------------------------------------------------------*/
static void reloadCounter(void)
{
  const IrqState state = irqSave();

  LPC_WDT->FEED = FEED_FIRST;
  LPC_WDT->FEED = FEED_SECOND;

  irqRestore(state);
}
/*----------------------------------------------------------------------------*/
static enum Result wdtInit(void *object, const void *configBase)
{
  const struct WdtConfig * const config = configBase;
  assert(config);

  const uint64_t clock = (((1ULL << 32) + 3999) / 4000) * wdtGetClock(object);
  const uint32_t timeout = (clock * config->period) >> 32;

  if (timeout > (0xFFFFFFFFUL >> (32 - WDT_TIMER_RESOLUTION)))
    return E_VALUE;

  const struct WdtBaseConfig baseConfig = {
      .source = config->source
  };
  struct Wdt * const timer = object;

  /* Call base class constructor */
  const enum Result res = WdtBase->init(timer, &baseConfig);
  if (res != E_OK)
    return res;

  timer->fired = (LPC_WDT->MOD & MOD_WDTOF) != 0;

  LPC_WDT->TC = timeout;
  LPC_WDT->MOD = MOD_WDEN | MOD_WDRESET | MOD_WDINT;

  const IrqState state = irqSave();

  /* Enable the counter */
  LPC_WDT->FEED = FEED_FIRST;
  LPC_WDT->FEED = FEED_SECOND;
  /* Reload the counter */
  LPC_WDT->FEED = FEED_FIRST;
  LPC_WDT->FEED = FEED_SECOND;

  irqRestore(state);
  return E_OK;
}
/*----------------------------------------------------------------------------*/
static bool wdtFired(const void *object)
{
  const struct Wdt * const timer = object;
  return timer->fired;
}
/*----------------------------------------------------------------------------*/
static void wdtReload(void *object __attribute__((unused)))
{
  reloadCounter();
}
