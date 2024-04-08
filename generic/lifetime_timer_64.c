/*
 * lifetime_timer_64.c
 * Copyright (C) 2019 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/generic/lifetime_timer_64.h>
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
static uint64_t tmrGetValue64(const void *);
static void tmrSetValue64(void *, uint64_t);
/*----------------------------------------------------------------------------*/
const struct Timer64Class * const LifetimeTimer64 =
    &(const struct Timer64Class){
    .base = {
        .size = sizeof(struct LifetimeTimer64),
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
    },

    .getValue64 = tmrGetValue64,
    .setValue64 = tmrSetValue64
};
/*----------------------------------------------------------------------------*/
static void onTimerOverflow(void *object)
{
  struct LifetimeTimer64 * const timer = object;
  const uint32_t overflow = timerGetOverflow(timer->timer);

  if (overflow)
    timer->ticks += (uint64_t)overflow;
  else
    timer->ticks += 1ULL << 32;
}
/*----------------------------------------------------------------------------*/
static enum Result tmrInit(void *object, const void *configBase)
{
  const struct LifetimeTimer64Config * const config = configBase;
  struct LifetimeTimer64 * const timer = object;

  timer->ticks = 0;
  timer->timer = config->timer;

  timerSetCallback(timer->timer, onTimerOverflow, timer);
  return E_OK;
}
/*----------------------------------------------------------------------------*/
static void tmrDeinit(void *object)
{
  struct LifetimeTimer64 * const timer = object;
  timerSetCallback(timer->timer, NULL, NULL);
}
/*----------------------------------------------------------------------------*/
static void tmrEnable(void *object)
{
  struct LifetimeTimer64 * const timer = object;
  timerEnable(timer->timer);
}
/*----------------------------------------------------------------------------*/
static void tmrDisable(void *object)
{
  struct LifetimeTimer64 * const timer = object;
  timerDisable(timer->timer);
}
/*----------------------------------------------------------------------------*/
static uint32_t tmrGetFrequency(const void *object)
{
  const struct LifetimeTimer64 * const timer = object;
  return timerGetFrequency(timer->timer);
}
/*----------------------------------------------------------------------------*/
static void tmrSetFrequency(void *object, uint32_t frequency)
{
  const struct LifetimeTimer64 * const timer = object;
  timerSetFrequency(timer->timer, frequency);
}
/*----------------------------------------------------------------------------*/
static uint32_t tmrGetValue(const void *object)
{
  return (uint32_t)tmrGetValue64(object);
}
/*----------------------------------------------------------------------------*/
static void tmrSetValue(void *object, uint32_t value)
{
  tmrSetValue64(object, (uint64_t)value);
}
/*----------------------------------------------------------------------------*/
static uint64_t tmrGetValue64(const void *object)
{
  const struct LifetimeTimer64 * const timer = object;
  uint64_t accumulated;
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
static void tmrSetValue64(void *object, [[maybe_unused]] uint64_t value)
{
  assert(value == 0);

  struct LifetimeTimer64 * const timer = object;

  do
  {
    timerSetValue(timer->timer, 0);
    timer->ticks = 0;
    barrier();
  }
  while (timer->ticks);
}
