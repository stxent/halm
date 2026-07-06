/*
 * timer.c
 * Copyright (C) 2020 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/platform/generic/timer.h>
#include <uv.h>
#include <assert.h>
#include <limits.h>
#include <pthread.h>
#include <stdlib.h>
/*----------------------------------------------------------------------------*/
#define TICK_RATE 1000

struct PosixTimer
{
  struct Timer base;

  void (*callback)(void *);
  void *callbackArgument;

  uv_timer_t *handle;

  uint64_t frequency;
  uint64_t overflow;
  uint64_t resolution;
  uint64_t timestamp;
  bool autostop;
  bool freerun;
};
/*----------------------------------------------------------------------------*/
static void onCloseCallback(uv_handle_t *);
static void onTimerCallback(uv_timer_t *);
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
const struct TimerClass * const Timer = &(const struct TimerClass){
    .size = sizeof(struct PosixTimer),
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
static void onCloseCallback(uv_handle_t *handle)
{
  free(handle);
}
/*----------------------------------------------------------------------------*/
static void onTimerCallback(uv_timer_t *handle)
{
  struct PosixTimer * const timer = uv_handle_get_data((uv_handle_t *)handle);
  bool event = false;

  if (timer->freerun)
  {
    if (++timer->timestamp > timer->resolution)
      timer->timestamp = 0;
    if (timer->timestamp == timer->overflow)
      event = true;
  }
  else
  {
    timer->timestamp = uv_now(uv_default_loop());
    event = true;
  }

  if (event && timer->callback != NULL)
    timer->callback(timer->callbackArgument);
}
/*----------------------------------------------------------------------------*/
static enum Result tmrInit(void *object, const void *configBase)
{
  const struct TimerConfig * const config = configBase;
  struct PosixTimer * const timer = object;

  timer->handle = malloc(sizeof(uv_timer_t));
  if (timer->handle == NULL)
    return E_MEMORY;

  if (uv_timer_init(uv_default_loop(), timer->handle) < 0)
  {
    free(timer->handle);
    return E_ERROR;
  }
  uv_handle_set_data((uv_handle_t *)timer->handle, timer);

  timer->callback = NULL;
  timer->callbackArgument = NULL;
  timer->autostop = false;

  if (config != NULL)
  {
    assert(config->frequency <= TICK_RATE);
    timer->frequency = config->frequency ? config->frequency : TICK_RATE;
    timer->resolution = config->resolution ?
        config->resolution - 1 : UINT32_MAX;
    timer->freerun = config->freerun;
  }
  else
  {
    timer->frequency = TICK_RATE;
    timer->resolution = UINT32_MAX;
    timer->freerun = false;
  }

  timer->overflow = timer->resolution;
  timer->timestamp = timer->freerun ? 0 : uv_now(uv_default_loop());
  return E_OK;
}
/*----------------------------------------------------------------------------*/
static void tmrDeinit(void *object)
{
  struct PosixTimer * const timer = object;

  uv_handle_set_data((uv_handle_t *)timer->handle, NULL);
  uv_close((uv_handle_t *)timer->handle, onCloseCallback);
}
/*----------------------------------------------------------------------------*/
static void tmrEnable(void *object)
{
  struct PosixTimer * const timer = object;
  uint64_t period;

  if (timer->freerun)
  {
    period = TICK_RATE / timer->frequency;
    timer->timestamp = 0;
  }
  else
  {
    period = (timer->overflow * TICK_RATE) / timer->frequency;
    timer->timestamp = uv_now(uv_default_loop());
  }

  uv_timer_start(timer->handle, onTimerCallback, period,
      timer->autostop ? 0 : period);
}
/*----------------------------------------------------------------------------*/
static void tmrDisable(void *object)
{
  struct PosixTimer * const timer = object;
  uv_timer_stop(timer->handle);
}
/*----------------------------------------------------------------------------*/
static void tmrSetAutostop(void *object, bool state)
{
  struct PosixTimer * const timer = object;

  assert(!state || !timer->freerun);
  timer->autostop = state;
}
/*----------------------------------------------------------------------------*/
static void tmrSetCallback(void *object, void (*callback)(void *),
    void *argument)
{
  struct PosixTimer * const timer = object;

  timer->callbackArgument = argument;
  timer->callback = callback;
}
/*----------------------------------------------------------------------------*/
static uint32_t tmrGetFrequency(const void *object)
{
  const struct PosixTimer * const timer = object;
  return (uint32_t)timer->frequency;
}
/*----------------------------------------------------------------------------*/
static void tmrSetFrequency(void *object, uint32_t frequency)
{
  struct PosixTimer * const timer = object;

  assert(frequency <= TICK_RATE);
  timer->frequency = frequency ? frequency : TICK_RATE;

  if (uv_is_active((uv_handle_t *)timer->handle))
  {
    const uint64_t period = (timer->overflow * TICK_RATE) / timer->frequency;
    uv_timer_set_repeat(timer->handle, period);
  }
}
/*----------------------------------------------------------------------------*/
static uint32_t tmrGetOverflow(const void *object)
{
  const struct PosixTimer * const timer = object;
  return (uint32_t)(timer->freerun ? timer->overflow + 1 : timer->overflow);
}
/*----------------------------------------------------------------------------*/
static void tmrSetOverflow(void *object, uint32_t overflow)
{
  struct PosixTimer * const timer = object;

  if (timer->freerun)
    overflow = overflow ? overflow - 1 : timer->resolution;
  else
    overflow = overflow ? overflow : timer->resolution;

  timer->overflow = overflow <= timer->resolution ?
      overflow : timer->resolution;

  if (!timer->freerun && uv_is_active((uv_handle_t *)timer->handle))
  {
    const uint64_t period = (timer->overflow * 1000) / timer->frequency;
    uv_timer_set_repeat(timer->handle, period);
  }
}
/*----------------------------------------------------------------------------*/
static uint32_t tmrGetValue(const void *object)
{
  const struct PosixTimer * const timer = object;

  if (timer->freerun)
    return timer->timestamp;
  else
    return (uint32_t)(uv_now(uv_default_loop()) - timer->timestamp);
}
/*----------------------------------------------------------------------------*/
static void tmrSetValue(void *object, uint32_t value)
{
  struct PosixTimer * const timer = object;

  assert(value <= timer->resolution);
  if (timer->freerun)
    timer->timestamp = value;
  else
    timer->timestamp = (uint32_t)uv_now(uv_default_loop()) - value;
}
