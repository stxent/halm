/*
 * wdt.c
 * Copyright (C) 2023 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/platform/numicro/system.h>
#include <halm/platform/numicro/wdt.h>
#include <halm/platform/numicro/wdt_defs.h>
#include <halm/platform/platform_defs.h>
#include <assert.h>
/*----------------------------------------------------------------------------*/
static void interruptHandler(void *);
/*----------------------------------------------------------------------------*/
static enum Result wdtInit(void *, const void *);
static bool wdtFired(const void *);
static void wdtSetCallback(void *, void (*)(void *), void *);
static void wdtReload(void *);
/*----------------------------------------------------------------------------*/
const struct WatchdogClass * const Wdt = &(const struct WatchdogClass){
    .size = sizeof(struct Wdt),
    .init = wdtInit,
    .deinit = 0, /* Default destructor */

    .fired = wdtFired,
    .setCallback = wdtSetCallback,
    .reload = wdtReload
};
/*----------------------------------------------------------------------------*/
static void interruptHandler(void *object)
{
  struct Wdt * const timer = object;

  /* Clear interrupt flags */
  NM_WDT->CTL = NM_WDT->CTL;

  if (timer->callback)
    timer->callback(timer->callbackArgument);
}
/*----------------------------------------------------------------------------*/
static enum Result wdtInit(void *object, const void *configBase)
{
  const struct WdtConfig * const config = configBase;
  assert(config);

  struct Wdt * const timer = object;

  /* Call base class constructor */
  const enum Result res = WdtBase->init(timer, 0);
  if (res != E_OK)
    return res;

  timer->base.handler = interruptHandler;
  timer->callback = 0;
  timer->fired = (NM_WDT->CTL & CTL_RSTF) != 0;

  const uint32_t clock = wdtGetClock(object);
  const uint32_t prescaler = config->period * clock / 1000;
  uint32_t ctl = CTL_RSTF | CTL_IF | CTL_WKEN | CTL_WKF | CTL_WDTEN;
  uint32_t interval = 4;

  while ((1UL << interval) < prescaler)
    interval += 2;
  assert(interval <= 18);

  if (!config->disarmed)
    ctl |= CTL_RSTEN;
  ctl |= CTL_TOUTSEL((interval - 4) >> 1);

  sysUnlockReg();
  /* Reset counter */
  NM_WDT->CTL = CTL_RSTCNT;
  while (NM_WDT->CTL & CTL_RSTCNT);

  /* Select minimal time-out window for reset */
  NM_WDT->ALTCTL = ALTCTL_RSTDSEL(RSTDSEL_3);
  /* Enable counter */
  NM_WDT->CTL = ctl;
  sysLockReg();

  irqSetPriority(timer->base.irq, config->priority);
  irqEnable(timer->base.irq);

  wdtReload(object);
  return E_OK;
}
/*----------------------------------------------------------------------------*/
static bool wdtFired(const void *object)
{
  const struct Wdt * const timer = object;
  return timer->fired;
}
/*----------------------------------------------------------------------------*/
static void wdtSetCallback(void *object, void (*callback)(void *),
    void *argument)
{
  struct Wdt * const timer = object;
  uint32_t ctl = NM_WDT->CTL & ~CTL_INTEN;

  timer->callbackArgument = argument;
  timer->callback = callback;

  if (timer->callback)
    ctl |= CTL_INTEN;

  sysUnlockReg();
  NM_WDT->CTL = ctl;
  sysLockReg();
}
/*----------------------------------------------------------------------------*/
static void wdtReload(void *object __attribute__((unused)))
{
  NM_WDT->RSTCNT = RSTCNT_MAGIC_NUMBER;
}
