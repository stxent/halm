/*
 * systick.c
 * Copyright (C) 2013 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/clock.h>
#include <halm/core/cortex/systick.h>
#include <halm/core/cortex/systick_defs.h>
#include <xcore/bits.h>
#include <assert.h>
/*----------------------------------------------------------------------------*/
static bool setInstance(struct SysTick *);
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
#  define tmrDeinit deletedDestructorTrap
#endif
/*----------------------------------------------------------------------------*/
const struct TimerClass * const SysTick = &(const struct TimerClass){
    .size = sizeof(struct SysTick),
    .init = tmrInit,
    .deinit = tmrDeinit,

    .enable = tmrEnable,
    .disable = tmrDisable,
    .setAutostop = NULL,
    .setCallback = tmrSetCallback,
    .getFrequency = tmrGetFrequency,
    .setFrequency = NULL,
    .getOverflow = tmrGetOverflow,
    .setOverflow = tmrSetOverflow,
    .getValue = tmrGetValue,
    .setValue = tmrSetValue
};
/*----------------------------------------------------------------------------*/
extern const struct ClockClass * const MainClock;
static struct SysTick *instance = NULL;
/*----------------------------------------------------------------------------*/
static bool setInstance(struct SysTick *object)
{
  if (instance == NULL)
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
  const struct SysTickConfig * const config = configBase;
  struct SysTick * const timer = object;

  if (setInstance(timer))
  {
    /* Configure the timer but leave it in the disabled state */
    SYSTICK->CTRL = CTRL_CLKSOURCE;
    SYSTICK->LOAD = TIMER_RESOLUTION;

    if (config != NULL)
      irqSetPriority(SYSTICK_IRQ, config->priority);

    return E_OK;
  }
  else
    return E_BUSY;
}
/*----------------------------------------------------------------------------*/
#ifndef CONFIG_CORE_CORTEX_SYSTICK_NO_DEINIT
static void tmrDeinit([[maybe_unused]] void *object)
{
  SYSTICK->CTRL = 0;
  instance = NULL;
}
#endif
/*----------------------------------------------------------------------------*/
static void tmrEnable([[maybe_unused]] void *object)
{
  SYSTICK->CTRL |= CTRL_ENABLE;
}
/*----------------------------------------------------------------------------*/
static void tmrDisable([[maybe_unused]] void *object)
{
  SYSTICK->CTRL &= ~CTRL_ENABLE;
}
/*----------------------------------------------------------------------------*/
static void tmrSetCallback(void *object, void (*callback)(void *),
    void *argument)
{
  struct SysTick * const timer = object;

  timer->callback = callback;
  timer->callbackArgument = argument;

  if (timer->callback != NULL)
    SYSTICK->CTRL |= CTRL_TICKINT;
  else
    SYSTICK->CTRL &= ~CTRL_TICKINT;
}
/*----------------------------------------------------------------------------*/
static uint32_t tmrGetFrequency([[maybe_unused]] const void *object)
{
  return clockFrequency(MainClock);
}
/*----------------------------------------------------------------------------*/
static uint32_t tmrGetOverflow([[maybe_unused]] const void *object)
{
  return SYSTICK->LOAD + 1;
}
/*----------------------------------------------------------------------------*/
static void tmrSetOverflow([[maybe_unused]] void *object, uint32_t overflow)
{
  const uint32_t state = SYSTICK->CTRL & ~CTRL_COUNTFLAG;

  assert(overflow <= TIMER_RESOLUTION);

  SYSTICK->CTRL = 0;
  SYSTICK->LOAD = (overflow - 1) & TIMER_RESOLUTION;
  SYSTICK->CTRL = state;
}
/*----------------------------------------------------------------------------*/
static uint32_t tmrGetValue([[maybe_unused]] const void *object)
{
  return SYSTICK->LOAD - SYSTICK->VAL;
}
/*----------------------------------------------------------------------------*/
static void tmrSetValue([[maybe_unused]] void *object, uint32_t value)
{
  assert(value <= SYSTICK->LOAD);
  SYSTICK->VAL = SYSTICK->LOAD - value;
}
