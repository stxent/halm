/*
 * systick.c
 * Copyright (C) 2013 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <assert.h>
#include <xcore/bits.h>
#include <halm/clock.h>
#include <halm/core/cortex/systick.h>
#include <halm/core/cortex/systick_defs.h>
/*----------------------------------------------------------------------------*/
static void resetInstance(void);
static bool setInstance(struct SysTickTimer *);
/*----------------------------------------------------------------------------*/
static enum Result tmrInit(void *, const void *);
static void tmrEnable(void *);
static void tmrDisable(void *);
static void tmrSetCallback(void *, void (*)(void *), void *);
static uint32_t tmrGetFrequency(const void *);
static uint32_t tmrGetOverflow(const void *);
static void tmrSetOverflow(void *, uint32_t);
static uint32_t tmrGetValue(const void *);
static void tmrSetValue(void *, uint32_t);

#ifndef CONFIG_CORE_CORTEX_SYSTICK_NO_DEINIT
static void tmrDeinit(void *);
#else
#define tmrDeinit deletedDestructorTrap
#endif
/*----------------------------------------------------------------------------*/
static const struct TimerClass tmrTable = {
    .size = sizeof(struct SysTickTimer),
    .init = tmrInit,
    .deinit = tmrDeinit,

    .enable = tmrEnable,
    .disable = tmrDisable,
    .setCallback = tmrSetCallback,
    .getFrequency = tmrGetFrequency,
    .setFrequency = 0,
    .getOverflow = tmrGetOverflow,
    .setOverflow = tmrSetOverflow,
    .getValue = tmrGetValue,
    .setValue = tmrSetValue
};
/*----------------------------------------------------------------------------*/
extern const struct ClockClass * const MainClock;
const struct TimerClass * const SysTickTimer = &tmrTable;
static struct SysTickTimer *instance = 0;
/*----------------------------------------------------------------------------*/
static void resetInstance(void)
{
  instance = 0;
}
/*----------------------------------------------------------------------------*/
static bool setInstance(struct SysTickTimer *object)
{
  if (!instance)
  {
    instance = object;
    return true;
  }
  else
    return false;
}
/*----------------------------------------------------------------------------*/
void SYSTICK_ISR(void)
{
  /* Reading of CTRL register will clear the COUNTFLAG bit */
  if (SYSTICK->CTRL & CTRL_COUNTFLAG)
    instance->callback(instance->callbackArgument);
}
/*----------------------------------------------------------------------------*/
static enum Result tmrInit(void *object, const void *configBase)
{
  const struct SysTickTimerConfig * const config = configBase;
  struct SysTickTimer * const timer = object;

  if (setInstance(timer))
  {
    /* Configure the timer but leave it in the disabled state */
    SYSTICK->CTRL = CTRL_CLKSOURCE;
    SYSTICK->LOAD = TIMER_RESOLUTION;

    if (config)
      irqSetPriority(SYSTICK_IRQ, config->priority);

    return E_OK;
  }
  else
    return E_BUSY;
}
/*----------------------------------------------------------------------------*/
#ifndef CONFIG_CORE_CORTEX_SYSTICK_NO_DEINIT
static void tmrDeinit(void *object __attribute__((unused)))
{
  SYSTICK->CTRL = 0;
  resetInstance();
}
#endif
/*----------------------------------------------------------------------------*/
static void tmrEnable(void *object __attribute__((unused)))
{
  SYSTICK->CTRL |= CTRL_ENABLE;
}
/*----------------------------------------------------------------------------*/
static void tmrDisable(void *object __attribute__((unused)))
{
  SYSTICK->CTRL &= ~CTRL_ENABLE;
}
/*----------------------------------------------------------------------------*/
static void tmrSetCallback(void *object, void (*callback)(void *),
    void *argument)
{
  struct SysTickTimer * const timer = object;

  timer->callback = callback;
  timer->callbackArgument = argument;

  if (timer->callback)
    SYSTICK->CTRL |= CTRL_TICKINT;
  else
    SYSTICK->CTRL &= ~CTRL_TICKINT;
}
/*----------------------------------------------------------------------------*/
static uint32_t tmrGetFrequency(const void *object __attribute__((unused)))
{
  return clockFrequency(MainClock);
}
/*----------------------------------------------------------------------------*/
static uint32_t tmrGetOverflow(const void *object __attribute__((unused)))
{
  return SYSTICK->LOAD + 1;
}
/*----------------------------------------------------------------------------*/
static void tmrSetOverflow(void *object __attribute__((unused)),
    uint32_t overflow)
{
  const uint32_t state = SYSTICK->CTRL & ~CTRL_COUNTFLAG;

  assert(overflow <= TIMER_RESOLUTION);

  SYSTICK->CTRL = 0;
  SYSTICK->LOAD = overflow ? overflow - 1 : TIMER_RESOLUTION;
  SYSTICK->CTRL = state;
}
/*----------------------------------------------------------------------------*/
static uint32_t tmrGetValue(const void *object __attribute__((unused)))
{
  return SYSTICK->VAL;
}
/*----------------------------------------------------------------------------*/
static void tmrSetValue(void *object __attribute__((unused)), uint32_t value)
{
  assert(value <= TIMER_RESOLUTION);
  SYSTICK->VAL = value;
}
