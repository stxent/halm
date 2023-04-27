/*
 * wwdt.c
 * Copyright (C) 2023 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <halm/platform/lpc/wdt_defs.h>
#include <halm/platform/lpc/wwdt.h>
#include <halm/platform/platform_defs.h>
#include <assert.h>
/*----------------------------------------------------------------------------*/
static void interruptHandler(void *);
static void reloadCounter(void);
/*----------------------------------------------------------------------------*/
static enum Result wdtInit(void *, const void *);
static bool wdtFired(const void *);
static void wdtSetCallback(void *, void (*)(void *), void *);
static void wdtReload(void *);
/*----------------------------------------------------------------------------*/
const struct WatchdogClass * const Wwdt = &(const struct WatchdogClass){
    .size = sizeof(struct Wwdt),
    .init = wdtInit,
    .deinit = NULL, /* Default destructor */

    .fired = wdtFired,
    .setCallback = wdtSetCallback,
    .reload = wdtReload
};
/*----------------------------------------------------------------------------*/
static void interruptHandler(void *object)
{
  struct Wwdt * const timer = object;
  const uint32_t mod = LPC_WWDT->MOD;

  if (mod & MOD_WDINT)
  {
    if (mod & MOD_WDTOF)
    {
      /* Interrupt Mode, clear flags and re-enable the counter */
      LPC_WWDT->MOD = (mod & ~MOD_WDTOF) | MOD_WDEN;
      /* Reload the counter manually */
      reloadCounter();
    }
    else
    {
      /* Reset Mode, clear pending interrupt flag */
      LPC_WWDT->MOD = mod;
    }

    if (timer->callback != NULL)
      timer->callback(timer->callbackArgument);
  }
}
/*----------------------------------------------------------------------------*/
static void reloadCounter(void)
{
  const IrqState state = irqSave();

  LPC_WWDT->FEED = FEED_FIRST;
  LPC_WWDT->FEED = FEED_SECOND;

  irqRestore(state);
}
/*----------------------------------------------------------------------------*/
static enum Result wdtInit(void *object, const void *configBase)
{
  const struct WwdtConfig * const config = configBase;
  assert(config != NULL);
  assert(!config->disarmed || !config->window);

  const uint64_t clock = (((1ULL << 32) + 3999) / 4000) * wdtGetClock(object);
  const uint32_t timeout = (clock * config->period) >> 32;
  const uint32_t window = (clock * config->window) >> 32;

  if (timeout >= (1 << 24))
    return E_VALUE;

  const struct WdtBaseConfig baseConfig = {
      .source = config->source
  };
  struct Wwdt * const timer = object;

  /* Call base class constructor */
  const enum Result res = WdtBase->init(timer, &baseConfig);
  if (res != E_OK)
    return res;

  timer->base.handler = interruptHandler;
  timer->callback = NULL;
  timer->fired = (LPC_WWDT->MOD & MOD_WDTOF) != 0;

  uint32_t mod = MOD_WDEN | MOD_WDINT;
  IrqState state;

  if (!config->disarmed)
    mod |= MOD_WDRESET | MOD_WDPROTECT;

  LPC_WWDT->TC = timeout;
  LPC_WWDT->MOD = mod;

  if (config->disarmed)
  {
    LPC_WWDT->WINDOW = (1 << 24) - 1;
    LPC_WWDT->WARNINT = 0;
  }
  else
  {
    LPC_WWDT->WINDOW = window;
    LPC_WWDT->WARNINT = MIN((1 << 10) - 1, window);
  }

  state = irqSave();
  /* Enable the counter */
  LPC_WWDT->FEED = FEED_FIRST;
  LPC_WWDT->FEED = FEED_SECOND;
  /* Reload the counter */
  LPC_WWDT->FEED = FEED_FIRST;
  LPC_WWDT->FEED = FEED_SECOND;
  irqRestore(state);

  irqSetPriority(timer->base.irq, config->priority);
  irqEnable(timer->base.irq);

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static bool wdtFired(const void *object)
{
  const struct Wwdt * const timer = object;
  return timer->fired;
}
/*----------------------------------------------------------------------------*/
static void wdtSetCallback(void *object, void (*callback)(void *),
    void *argument)
{
  struct Wwdt * const timer = object;

  timer->callbackArgument = argument;
  timer->callback = callback;
}
/*----------------------------------------------------------------------------*/
static void wdtReload(void *object __attribute__((unused)))
{
  reloadCounter();
}
