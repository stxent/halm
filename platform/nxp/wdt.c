/*
 * wdt.c
 * Copyright (C) 2014 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <halm/platform/nxp/wdt.h>
#include <halm/platform/nxp/wdt_defs.h>
#include <halm/platform/platform_defs.h>
/*----------------------------------------------------------------------------*/
static void interruptHandler(void *);
/*----------------------------------------------------------------------------*/
static enum result wdtInit(void *, const void *);
static void wdtDeinit(void *);
static void wdtCallback(void *, void (*)(void *), void *);
static void wdtRestart(void *);
/*----------------------------------------------------------------------------*/
static const struct WatchdogClass wdtTable = {
    .size = sizeof(struct Wdt),
    .init = wdtInit,
    .deinit = wdtDeinit,

    .callback = wdtCallback,
    .restart = wdtRestart
};
/*----------------------------------------------------------------------------*/
const struct WatchdogClass * const Wdt = &wdtTable;
/*----------------------------------------------------------------------------*/
static void interruptHandler(void *object)
{
  struct Wdt * const timer = object;

  if (timer->callback)
    timer->callback(timer->callbackArgument);
}
/*----------------------------------------------------------------------------*/
static enum result wdtInit(void *object, const void *configBase)
{
  const struct WdtConfig * const config = configBase;
  const struct WdtBaseConfig baseConfig = {
      .source = config->source
  };
  struct Wdt * const timer = object;
  enum result res;

  /* Call base class constructor */
  if ((res = WdtBase->init(object, &baseConfig)) != E_OK)
    return res;

  timer->base.handler = interruptHandler;
  timer->callback = 0;

  LPC_WDT->TC = (config->period * (wdtGetClock(object) / 1000)) >> 2;
  LPC_WDT->MOD = MOD_WDEN | MOD_WDRESET;

  irqSetPriority(timer->base.irq, config->priority);

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static void wdtDeinit(void *object __attribute__((unused)))
{
  /* Watchdog timer cannot be disabled */
}
/*----------------------------------------------------------------------------*/
static void wdtCallback(void *object, void (*callback)(void *), void *argument)
{
  struct Wdt * const timer = object;

  timer->callbackArgument = argument;
  timer->callback = callback;

  if (timer->callback)
    irqEnable(timer->base.irq);
  else
    irqDisable(timer->base.irq);
}
/*----------------------------------------------------------------------------*/
static void wdtRestart(void *object __attribute__((unused)))
{
  LPC_WDT->FEED = FEED_FIRST;
  LPC_WDT->FEED = FEED_SECOND;
}
