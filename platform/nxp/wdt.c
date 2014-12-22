/*
 * wdt.c
 * Copyright (C) 2014 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <platform/platform_defs.h>
#include <platform/nxp/wdt.h>
#include <platform/nxp/wdt_defs.h>
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
  struct Wdt * const timer = object;
  enum result res;

  /* Call base class constructor */
  if ((res = WdtBase->init(object, 0)) != E_OK)
    return res;

  timer->parent.handler = interruptHandler;
  timer->callback = 0;

  LPC_WDT->TC = (config->period * (wdtGetClock(object) / 1000)) >> 2;
  LPC_WDT->MOD = MOD_WDEN | MOD_WDRESET;

  irqSetPriority(timer->parent.irq, config->priority);

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
    irqEnable(timer->parent.irq);
  else
    irqDisable(timer->parent.irq);
}
/*----------------------------------------------------------------------------*/
static void wdtRestart(void *object __attribute__((unused)))
{
  LPC_WDT->FEED = FEED_FIRST;
  LPC_WDT->FEED = FEED_SECOND;
}
