/*
 * systick.c
 * Copyright (C) 2013 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <bits.h>
#include <clock.h>
#include <memory.h>
#include <core/cortex/systick.h>
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
static enum result setDescriptor(const struct SysTickTimer *,
    struct SysTickTimer *);
static void updateInterrupt(struct SysTickTimer *);
static enum result updateFrequency(struct SysTickTimer *, uint32_t, uint32_t);
/*----------------------------------------------------------------------------*/
static enum result tmrInit(void *, const void *);
static void tmrDeinit(void *);
static void tmrCallback(void *, void (*)(void *), void *);
static void tmrSetEnabled(void *, bool);
static enum result tmrSetFrequency(void *, uint32_t);
static enum result tmrSetOverflow(void *, uint32_t);
static enum result tmrSetValue(void *, uint32_t);
static uint32_t tmrValue(const void *);
/*----------------------------------------------------------------------------*/
static const struct TimerClass timerTable = {
    .size = sizeof(struct SysTickTimer),
    .init = tmrInit,
    .deinit = tmrDeinit,

    .callback = tmrCallback,
    .setEnabled = tmrSetEnabled,
    .setFrequency = tmrSetFrequency,
    .setOverflow = tmrSetOverflow,
    .setValue = tmrSetValue,
    .value = tmrValue
};
/*----------------------------------------------------------------------------*/
extern const struct ClockClass * const MainClock;
const struct TimerClass * const SysTickTimer = &timerTable;
static struct SysTickTimer *descriptor = 0;
/*----------------------------------------------------------------------------*/
static void interruptHandler(void *object)
{
  struct SysTickTimer * const device = object;

  if ((SYSTICK->CTRL & CTRL_COUNTFLAG) && device->callback)
    device->callback(device->callbackArgument);
}
/*----------------------------------------------------------------------------*/
static enum result setDescriptor(const struct SysTickTimer *state,
    struct SysTickTimer *timer)
{
  return compareExchangePointer((void **)&descriptor, state,
      timer) ? E_OK : E_BUSY;
}
/*----------------------------------------------------------------------------*/
static void updateInterrupt(struct SysTickTimer *timer)
{
  if (timer->overflow && timer->callback)
  {
    (void)SYSTICK->CTRL; /* Clear pending interrupt */
    SYSTICK->CTRL |= CTRL_TICKINT;
  }
  else
    SYSTICK->CTRL &= ~CTRL_TICKINT;
}
/*----------------------------------------------------------------------------*/
static enum result updateFrequency(struct SysTickTimer *timer,
    uint32_t frequency, uint32_t overflow)
{
  if (overflow > TIMER_RESOLUTION)
    return E_VALUE;

  const uint32_t limit = overflow ? overflow : 1;

  if (frequency)
  {
    const uint32_t divisor =
        (clockFrequency(MainClock) / frequency) * limit - 1;

    if (divisor > TIMER_RESOLUTION)
      return E_VALUE;

    SYSTICK->LOAD = divisor;
  }
  else
    SYSTICK->LOAD = limit - 1;

  /* Reset current timer value */
  SYSTICK->VAL = 0;

  timer->frequency = frequency;
  timer->overflow = overflow;

  return E_OK;
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
  struct SysTickTimer * const timer = object;
  enum result res;

  /* Try to set peripheral descriptor */
  if ((res = setDescriptor(0, timer)) != E_OK)
    return res;

  timer->handler = interruptHandler;

  SYSTICK->CTRL = 0; /* Stop the timer */

  if ((res = updateFrequency(timer, config->frequency, 1)) != E_OK)
    return res;

  SYSTICK->CTRL = CTRL_ENABLE | CTRL_CLKSOURCE;
  if (!config->disabled)
    SYSTICK->CTRL |= CTRL_ENABLE;

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
static void tmrCallback(void *object, void (*callback)(void *), void *argument)
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
    SYSTICK->CTRL |= CTRL_ENABLE;
  else
    SYSTICK->CTRL &= ~CTRL_ENABLE;
}
/*----------------------------------------------------------------------------*/
static enum result tmrSetFrequency(void *object, uint32_t frequency)
{
  struct SysTickTimer * const timer = object;
  const uint32_t state = SYSTICK->CTRL & CTRL_ENABLE;
  enum result res;

  SYSTICK->CTRL &= ~CTRL_ENABLE;
  res = updateFrequency(timer, frequency, timer->overflow);
  SYSTICK->CTRL |= state;

  return res;
}
/*----------------------------------------------------------------------------*/
static enum result tmrSetOverflow(void *object, uint32_t overflow)
{
  struct SysTickTimer * const timer = object;
  const uint32_t state = SYSTICK->CTRL & CTRL_ENABLE;
  enum result res;

  SYSTICK->CTRL &= ~CTRL_ENABLE;
  if ((res = updateFrequency(timer, timer->frequency, overflow)) == E_OK)
    updateInterrupt(timer);
  SYSTICK->CTRL |= state;

  return res;
}
/*----------------------------------------------------------------------------*/
static enum result tmrSetValue(void *object __attribute__((unused)),
    uint32_t value)
{
  SYSTICK->VAL = value;
  return E_OK;
}
/*----------------------------------------------------------------------------*/
static uint32_t tmrValue(const void *object __attribute__((unused)))
{
  return SYSTICK->VAL;
}
