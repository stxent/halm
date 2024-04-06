/*
 * lifetime_timer_32.c
 * Copyright (C) 2019 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/generic/lifetime_timer_32.h>
#include <xcore/asm.h>
#include <assert.h>
/*----------------------------------------------------------------------------*/
static void onTimerOverflow(void *);
/*----------------------------------------------------------------------------*/
static enum Result tmrInit(void *, const void *);
static void tmrDeinit(void *);
static void tmrEnable(void *);
static void tmrDisable(void *);
static uint32_t tmrGetFrequency(const void *);
static void tmrSetFrequency(void *, uint32_t);
static uint32_t tmrGetValue(const void *);
static void tmrSetValue(void *, uint32_t);
/*----------------------------------------------------------------------------*/
const struct TimerClass * const LifetimeTimer32 = &(const struct TimerClass){
    .size = sizeof(struct LifetimeTimer32),
    .init = tmrInit,
    .deinit = tmrDeinit,

    .enable = tmrEnable,
    .disable = tmrDisable,
    .setAutostop = NULL,
    .setCallback = NULL,
    .getFrequency = tmrGetFrequency,
    .setFrequency = tmrSetFrequency,
    .getOverflow = NULL,
    .setOverflow = NULL,
    .getValue = tmrGetValue,
    .setValue = tmrSetValue
};
/*----------------------------------------------------------------------------*/
static void onTimerOverflow(void *object)
{
  struct LifetimeTimer32 * const timer = object;
  timer->ticks += timerGetOverflow(timer->timer);
}
/*----------------------------------------------------------------------------*/
static enum Result tmrInit(void *object, const void *configBase)
{
  const struct LifetimeTimer32Config * const config = configBase;
  struct LifetimeTimer32 * const timer = object;

  timer->ticks = 0;
  timer->timer = config->timer;

  timerSetCallback(timer->timer, onTimerOverflow, timer);
  return E_OK;
}
/*----------------------------------------------------------------------------*/
static void tmrDeinit(void *object)
{
  struct LifetimeTimer32 * const timer = object;
  timerSetCallback(timer->timer, NULL, NULL);
}
/*----------------------------------------------------------------------------*/
static void tmrEnable(void *object)
{
  struct LifetimeTimer32 * const timer = object;
  timerEnable(timer->timer);
}
/*----------------------------------------------------------------------------*/
static void tmrDisable(void *object)
{
  struct LifetimeTimer32 * const timer = object;
  timerDisable(timer->timer);
}
/*----------------------------------------------------------------------------*/
static uint32_t tmrGetFrequency(const void *object)
{
  const struct LifetimeTimer32 * const timer = object;
  return timerGetFrequency(timer->timer);
}
/*----------------------------------------------------------------------------*/
static void tmrSetFrequency(void *object, uint32_t frequency)
{
  const struct LifetimeTimer32 * const timer = object;
  timerSetFrequency(timer->timer, frequency);
}
/*----------------------------------------------------------------------------*/
static uint32_t tmrGetValue(const void *object)
{
  const struct LifetimeTimer32 * const timer = object;
  uint32_t accumulated;
  uint32_t current;

  do
  {
    accumulated = timer->ticks;
    current = timerGetValue(timer->timer);
    barrier();
  }
  while (accumulated != timer->ticks);

  return accumulated + current;
}
/*----------------------------------------------------------------------------*/
static void tmrSetValue(void *object, uint32_t value)
{
  assert(value == 0);

  struct LifetimeTimer32 * const timer = object;

  do
  {
    timerSetValue(timer->timer, 0);
    timer->ticks = 0;
    barrier();
  }
  while (timer->ticks);
}
