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
static enum Result wdtInit(void *, const void *);
static void wdtSetCallback(void *, void (*)(void *), void *);
static void wdtReload(void *);
/*----------------------------------------------------------------------------*/
const struct WatchdogClass * const Wdt = &(const struct WatchdogClass){
    .size = sizeof(struct Wdt),
    .init = wdtInit,
    .deinit = 0, /* Default destructor */

    .setCallback = wdtSetCallback,
    .reload = wdtReload
};
/*----------------------------------------------------------------------------*/
static enum Result wdtInit(void *object, const void *configBase)
{
  const struct WdtConfig * const config = configBase;
  assert(config);

  const struct WdtBaseConfig baseConfig = {
      .source = config->source
  };
  enum Result res;

  /* Call base class constructor */
  if ((res = WdtBase->init(object, &baseConfig)) != E_OK)
    return res;

  const uint32_t clock = wdtGetClock(object) / 4;
  const uint32_t prescaler = config->period * (clock / 1000);

  assert(prescaler >= 1 << 8);
  assert(prescaler <= 0xFFFFFFFFUL >> (32 - WDT_TIMER_RESOLUTION));

  LPC_WDT->TC = prescaler;
  LPC_WDT->MOD = MOD_WDEN | MOD_WDRESET;

  wdtReload(object);

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static void wdtSetCallback(void *object __attribute__((unused)),
    void (*callback)(void *) __attribute__((unused)),
    void *argument __attribute__((unused)))
{
  /*
   * The main purpose of the WDT interrupt is to allow debugging.
   * This interrupt can not be used as a regular timer interrupt.
   */
}
/*----------------------------------------------------------------------------*/
static void wdtReload(void *object __attribute__((unused)))
{
  const IrqState state = irqSave();

  LPC_WDT->FEED = FEED_FIRST;
  LPC_WDT->FEED = FEED_SECOND;

  irqRestore(state);
}
