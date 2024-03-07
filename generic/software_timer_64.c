/*
 * software_timer_64.c
 * Copyright (C) 2019 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/generic/software_timer_64.h>
#include <xcore/asm.h>
#include <assert.h>
/*----------------------------------------------------------------------------*/
static void onTimerOverflow(void *);
/*----------------------------------------------------------------------------*/
static enum Result tmrInit(void *, const void *);
static void tmrDeinit(void *);
static void tmrEnable(void *);
static void tmrDisable(void *);
static void tmrSetAutostop(void *, bool);
static void tmrSetCallback(void *, void (*)(void *), void *);
static uint32_t tmrGetFrequency(const void *);
static void tmrSetFrequency(void *, uint32_t);
static uint32_t tmrGetOverflow(const void *);
static void tmrSetOverflow(void *, uint32_t);
static uint32_t tmrGetValue(const void *);
static void tmrSetValue(void *, uint32_t);
static uint64_t tmrGetValue64(const void *);
static void tmrSetValue64(void *, uint64_t);
/*----------------------------------------------------------------------------*/
const struct Timer64Class * const SoftwareTimer64 =
    &(const struct Timer64Class){
    .base = {
        .size = sizeof(struct SoftwareTimer64),
        .init = tmrInit,
        .deinit = tmrDeinit,

        .enable = tmrEnable,
        .disable = tmrDisable,
        .setAutostop = tmrSetAutostop,
        .setCallback = tmrSetCallback,
        .getFrequency = tmrGetFrequency,
        .setFrequency = tmrSetFrequency,
        .getOverflow = tmrGetOverflow,
        .setOverflow = tmrSetOverflow,
        .getValue = tmrGetValue,
        .setValue = tmrSetValue
    },

    .getValue64 = tmrGetValue64,
    .setValue64 = tmrSetValue64
};
/*----------------------------------------------------------------------------*/
static void onTimerOverflow(void *object)
{
  struct SoftwareTimer64 * const timer = object;
  const uint64_t previous = timer->ticks;
  const uint32_t overflow = timerGetOverflow(timer->timer);

  if (overflow)
    timer->ticks += (uint64_t)overflow;
  else
    timer->ticks += 1ULL << 32;

  if (timer->ticks <= previous && timer->callback)
    timer->callback(timer->callbackArgument);
}
/*----------------------------------------------------------------------------*/
static enum Result tmrInit(void *object, const void *configBase)
{
  const struct SoftwareTimer64Config * const config = configBase;
  struct SoftwareTimer64 * const timer = object;

  timer->callback = NULL;
  timer->ticks = 0;
  timer->timer = config->timer;

  timerSetCallback(timer->timer, onTimerOverflow, timer);
  return E_OK;
}
/*----------------------------------------------------------------------------*/
static void tmrDeinit(void *object)
{
  struct SoftwareTimer64 * const timer = object;
  timerSetCallback(timer->timer, NULL, NULL);
}
/*----------------------------------------------------------------------------*/
static void tmrEnable(void *object)
{
  struct SoftwareTimer64 * const timer = object;
  timerEnable(timer->timer);
}
/*----------------------------------------------------------------------------*/
static void tmrDisable(void *object)
{
  struct SoftwareTimer64 * const timer = object;
  timerDisable(timer->timer);
}
/*----------------------------------------------------------------------------*/
static void tmrSetAutostop(void *object, bool state)
{
  struct SoftwareTimer64 * const timer = object;

  // TODO
  (void)timer;
  (void)state;
}
/*----------------------------------------------------------------------------*/
static void tmrSetCallback(void *object, void (*callback)(void *),
    void *argument)
{
  struct SoftwareTimer64 * const timer = object;

  timer->callbackArgument = argument;
  timer->callback = callback;
}
/*----------------------------------------------------------------------------*/
static uint32_t tmrGetFrequency(const void *object)
{
  const struct SoftwareTimer64 * const timer = object;
  return timerGetFrequency(timer->timer);
}
/*----------------------------------------------------------------------------*/
static void tmrSetFrequency(void *object, uint32_t frequency)
{
  const struct SoftwareTimer64 * const timer = object;
  timerSetFrequency(timer->timer, frequency);
}
/*----------------------------------------------------------------------------*/
static uint32_t tmrGetOverflow([[maybe_unused]] const void *object)
{
  // TODO
  return 0;
}
/*----------------------------------------------------------------------------*/
static void tmrSetOverflow([[maybe_unused]] void *object,
    [[maybe_unused]] uint32_t overflow)
{
  // TODO
}
/*----------------------------------------------------------------------------*/
static uint32_t tmrGetValue(const void *object)
{
  const struct SoftwareTimer64 * const timer = object;
  return timerGetValue(timer->timer);
}
/*----------------------------------------------------------------------------*/
static void tmrSetValue(void *object, uint32_t value)
{
  struct SoftwareTimer64 * const timer = object;

  do
  {
    timerSetValue(timer->timer, 0);
    timer->ticks = value;
    barrier();
  }
  while (timer->ticks != value);
}
/*----------------------------------------------------------------------------*/
static uint64_t tmrGetValue64(const void *object)
{
  const struct SoftwareTimer64 * const timer = object;

  uint64_t accumulated;
  uint32_t current;

  do
  {
    accumulated = timer->ticks;
    current = /*timerGetOverflow(timer->timer) - */timerGetValue(timer->timer);
    barrier();
  }
  while (accumulated != timer->ticks);

  return accumulated + (uint64_t)current;
}
/*----------------------------------------------------------------------------*/
static void tmrSetValue64(void *object, uint64_t value)
{
  struct SoftwareTimer64 * const timer = object;

  do
  {
    timerSetValue(timer->timer, 0);
    timer->ticks = value;
    barrier();
  }
  while (timer->ticks != value);
}
