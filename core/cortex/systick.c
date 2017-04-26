/*
 * systick.c
 * Copyright (C) 2013 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <assert.h>
#include <xcore/bits.h>
#include <xcore/memory.h>
#include <halm/clock.h>
#include <halm/core/cortex/systick.h>
/*----------------------------------------------------------------------------*/
#define TIMER_RESOLUTION                ((1UL << 24) - 1)
/*----------------------------------------------------------------------------*/
/* System Tick counter enable */
#define CTRL_ENABLE                     BIT(0)
/* Interrupt enable */
#define CTRL_TICKINT                    BIT(1)
/* Clock source selection: 0 for external clock, 1 for processor clock */
#define CTRL_CLKSOURCE                  BIT(2)
/* Counter flag is set when counter counts down to zero */
#define CTRL_COUNTFLAG                  BIT(16)
/*----------------------------------------------------------------------------*/
static void interruptHandler(void *);
static bool setDescriptor(const struct SysTickTimer *, struct SysTickTimer *);
static void updateInterrupt(struct SysTickTimer *);
static void updateFrequency(struct SysTickTimer *, uint32_t, uint32_t);
/*----------------------------------------------------------------------------*/
static enum result tmrInit(void *, const void *);
static void tmrDeinit(void *);
static void tmrSetCallback(void *, void (*)(void *), void *);
static void tmrSetEnabled(void *, bool);
static uint32_t tmrGetFrequency(const void *);
static void tmrSetFrequency(void *, uint32_t);
static uint32_t tmrGetOverflow(const void *);
static void tmrSetOverflow(void *, uint32_t);
static uint32_t tmrGetValue(const void *);
static void tmrSetValue(void *, uint32_t);
/*----------------------------------------------------------------------------*/
static const struct TimerClass tmrTable = {
    .size = sizeof(struct SysTickTimer),
    .init = tmrInit,
    .deinit = tmrDeinit,

    .setCallback = tmrSetCallback,
    .setEnabled = tmrSetEnabled,
    .getFrequency = tmrGetFrequency,
    .setFrequency = tmrSetFrequency,
    .getOverflow = tmrGetOverflow,
    .setOverflow = tmrSetOverflow,
    .getValue = tmrGetValue,
    .setValue = tmrSetValue
};
/*----------------------------------------------------------------------------*/
extern const struct ClockClass * const MainClock;
const struct TimerClass * const SysTickTimer = &tmrTable;
static struct SysTickTimer *descriptor = 0;
/*----------------------------------------------------------------------------*/
static void interruptHandler(void *object)
{
  struct SysTickTimer * const device = object;

  if ((SYSTICK->CTRL & CTRL_COUNTFLAG) && device->callback)
    device->callback(device->callbackArgument);
}
/*----------------------------------------------------------------------------*/
static bool setDescriptor(const struct SysTickTimer *state,
    struct SysTickTimer *timer)
{
  return compareExchangePointer((void **)&descriptor, state, timer);
}
/*----------------------------------------------------------------------------*/
static void updateInterrupt(struct SysTickTimer *timer)
{
  if (timer->overflow && timer->callback)
  {
    /* Pending interrupt will be cleared during register modification */
    SYSTICK->CTRL |= CTRL_TICKINT;
  }
  else
  {
    SYSTICK->CTRL &= ~CTRL_TICKINT;
  }
}
/*----------------------------------------------------------------------------*/
static void updateFrequency(struct SysTickTimer *timer, uint32_t frequency,
    uint32_t overflow)
{
  assert(overflow <= TIMER_RESOLUTION);

  const uint32_t limit = overflow ? overflow : 1;

  if (frequency)
  {
    const uint32_t divisor =
        (clockFrequency(MainClock) / frequency) * limit - 1;

    assert(divisor <= TIMER_RESOLUTION);

    SYSTICK->LOAD = divisor;
  }
  else
    SYSTICK->LOAD = limit - 1;

  timer->frequency = frequency;
  timer->overflow = overflow;
}
/*----------------------------------------------------------------------------*/
void SYSTICK_ISR(void)
{
  descriptor->handler(descriptor);
}
/*----------------------------------------------------------------------------*/
static enum result tmrInit(void *object, const void *configBase)
{
  const struct SysTickTimerConfig * const config = configBase;
  assert(config);

  struct SysTickTimer * const timer = object;

  /* Try to set peripheral descriptor */
  if (!setDescriptor(0, timer))
    return E_BUSY;

  timer->handler = interruptHandler;

  SYSTICK->CTRL = 0; /* Stop the timer */

  updateFrequency(timer, config->frequency, 1);

  /* Timer is left disabled by default */
  SYSTICK->CTRL = CTRL_ENABLE | CTRL_CLKSOURCE;

  irqSetPriority(SYSTICK_IRQ, config->priority);
  irqEnable(SYSTICK_IRQ);

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static void tmrDeinit(void *object __attribute__((unused)))
{
  const struct SysTickTimer * const timer = object;

  irqDisable(SYSTICK_IRQ);
  SYSTICK->CTRL &= ~CTRL_ENABLE;
  setDescriptor(timer, 0);
}
/*----------------------------------------------------------------------------*/
static void tmrSetCallback(void *object, void (*callback)(void *),
    void *argument)
{
  struct SysTickTimer * const timer = object;

  timer->callback = callback;
  timer->callbackArgument = argument;

  updateInterrupt(timer);
}
/*----------------------------------------------------------------------------*/
static void tmrSetEnabled(void *object __attribute__((unused)), bool state)
{
  if (state)
  {
    SYSTICK->CTRL |= CTRL_ENABLE;
  }
  else
  {
    SYSTICK->CTRL &= ~CTRL_ENABLE;

    /* Clear possible pending interrupt */
    (void)SYSTICK->CTRL;
  }
}
/*----------------------------------------------------------------------------*/
static uint32_t tmrGetFrequency(const void *object)
{
  const struct SysTickTimer * const timer = object;
  const uint32_t baseClock = clockFrequency(MainClock);

  if (timer->frequency)
    return baseClock / (SYSTICK->LOAD + 1);
  else
    return baseClock;
}
/*----------------------------------------------------------------------------*/
static void tmrSetFrequency(void *object, uint32_t frequency)
{
  struct SysTickTimer * const timer = object;
  const uint32_t state = SYSTICK->CTRL & CTRL_ENABLE;

  SYSTICK->CTRL &= ~CTRL_ENABLE;
  updateFrequency(timer, frequency, timer->overflow);
  SYSTICK->CTRL |= state;
}
/*----------------------------------------------------------------------------*/
static uint32_t tmrGetOverflow(const void *object)
{
  const struct SysTickTimer * const timer = object;

  return timer->overflow;
}
/*----------------------------------------------------------------------------*/
static void tmrSetOverflow(void *object, uint32_t overflow)
{
  struct SysTickTimer * const timer = object;
  const uint32_t state = SYSTICK->CTRL & CTRL_ENABLE;

  SYSTICK->CTRL &= ~CTRL_ENABLE;
  updateFrequency(timer, timer->frequency, overflow);
  updateInterrupt(timer);
  SYSTICK->CTRL |= state;
}
/*----------------------------------------------------------------------------*/
static uint32_t tmrGetValue(const void *object __attribute__((unused)))
{
  return SYSTICK->VAL;
}
/*----------------------------------------------------------------------------*/
static void tmrSetValue(void *object __attribute__((unused)), uint32_t value)
{
  SYSTICK->VAL = value;
}
