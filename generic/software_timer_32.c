/*
 * software_timer_32.c
 * Copyright (C) 2019 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/generic/software_timer_32.h>
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
/*----------------------------------------------------------------------------*/
const struct TimerClass * const SoftwareTimer32 = &(const struct TimerClass){
    .size = sizeof(struct SoftwareTimer32),
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
};
/*----------------------------------------------------------------------------*/
static void onTimerOverflow(void *object)
{
  struct SoftwareTimer32 * const timer = object;
  const uint32_t previous = timer->ticks;

  timer->ticks += timerGetOverflow(timer->timer);

  if (timer->ticks <= previous && timer->callback)
    timer->callback(timer->callbackArgument);
}
/*----------------------------------------------------------------------------*/
static enum Result tmrInit(void *object, const void *configBase)
{
  const struct SoftwareTimer32Config * const config = configBase;
  struct SoftwareTimer32 * const timer = object;

  timer->callback = NULL;
  timer->ticks = 0;
  timer->timer = config->timer;

  timerSetCallback(timer->timer, onTimerOverflow, timer);
  return E_OK;
}
/*----------------------------------------------------------------------------*/
static void tmrDeinit(void *object)
{
  struct SoftwareTimer32 * const timer = object;
  timerSetCallback(timer->timer, NULL, NULL);
}
/*----------------------------------------------------------------------------*/
static void tmrEnable(void *object)
{
  struct SoftwareTimer32 * const timer = object;
  timerEnable(timer->timer);
}
/*----------------------------------------------------------------------------*/
static void tmrDisable(void *object)
{
  struct SoftwareTimer32 * const timer = object;
  timerDisable(timer->timer);
}
/*----------------------------------------------------------------------------*/
static void tmrSetAutostop(void *object, bool state)
{
  struct SoftwareTimer32 * const timer = object;

  // TODO
  (void)timer;
  (void)state;
}
/*----------------------------------------------------------------------------*/
static void tmrSetCallback(void *object, void (*callback)(void *),
    void *argument)
{
  struct SoftwareTimer32 * const timer = object;

  timer->callbackArgument = argument;
  timer->callback = callback;
}
/*----------------------------------------------------------------------------*/
static uint32_t tmrGetFrequency(const void *object)
{
  const struct SoftwareTimer32 * const timer = object;
  return timerGetFrequency(timer->timer);
}
/*----------------------------------------------------------------------------*/
static void tmrSetFrequency(void *object, uint32_t frequency)
{
  const struct SoftwareTimer32 * const timer = object;
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
  const struct SoftwareTimer32 * const timer = object;
  uint32_t accumulated;
  uint32_t current;

  do
  {
    accumulated = timer->ticks;
    current = /*timerGetOverflow(timer->timer) - */timerGetValue(timer->timer);
    barrier();
  }
  while (accumulated != timer->ticks);

  return accumulated + current;
}
/*----------------------------------------------------------------------------*/
static void tmrSetValue(void *object, uint32_t value)
{
  struct SoftwareTimer32 * const timer = object;

  do
  {
    timerSetValue(timer->timer, 0);
    timer->ticks = value;
    barrier();
  }
  while (timer->ticks != value);
}
