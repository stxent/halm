/*
 * mrt.c
 * Copyright (C) 2025 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/platform/lpc/mrt.h>
#include <halm/platform/lpc/mrt_defs.h>
#include <halm/platform/platform_defs.h>
#include <assert.h>
/*----------------------------------------------------------------------------*/
static void interruptHandler(void *);
/*----------------------------------------------------------------------------*/
static enum Result tmrInit(void *, const void *);
static void tmrEnable(void *);
static void tmrDisable(void *);
static void tmrSetAutostop(void *, bool);
static void tmrSetCallback(void *, void (*)(void *), void *);
static uint32_t tmrGetFrequency(const void *);
static uint32_t tmrGetOverflow(const void *);
static void tmrSetOverflow(void *, uint32_t);
static uint32_t tmrGetValue(const void *);
static void tmrSetValue(void *, uint32_t);

#ifndef CONFIG_PLATFORM_LPC_MRT_NO_DEINIT
static void tmrDeinit(void *);
#else
#  define tmrDeinit deletedDestructorTrap
#endif
/*----------------------------------------------------------------------------*/
const struct TimerClass * const Mrt = &(const struct TimerClass){
    .size = sizeof(struct Mrt),
    .init = tmrInit,
    .deinit = tmrDeinit,

    .enable = tmrEnable,
    .disable = tmrDisable,
    .setAutostop = tmrSetAutostop,
    .setCallback = tmrSetCallback,
    .getFrequency = tmrGetFrequency,
    .setFrequency = NULL,
    .getOverflow = tmrGetOverflow,
    .setOverflow = tmrSetOverflow,
    .getValue = tmrGetValue,
    .setValue = tmrSetValue
};
/*----------------------------------------------------------------------------*/
static void interruptHandler(void *object)
{
  struct Mrt * const timer = object;
  timer->callback(timer->callbackArgument);
}
/*----------------------------------------------------------------------------*/
static enum Result tmrInit(void *object, const void *configBase)
{
  const struct MrtConfig * const config = configBase;
  assert(config != NULL);

  const struct MrtBaseConfig baseConfig = {
      .channel = config->channel
  };
  struct Mrt * const timer = object;
  enum Result res;

  /* Call base class constructor */
  if ((res = MrtBase->init(timer, &baseConfig)) != E_OK)
    return res;

  timer->base.handler = interruptHandler;
  timer->callback = NULL;
  timer->interval = TIMER_RESOLUTION;

  LPC_MRT_Type * const reg = timer->base.reg;

  reg->CHANNEL[timer->base.channel].INTVAL = INTVAL_LOAD;
  reg->CHANNEL[timer->base.channel].CTRL = 0;

  return E_OK;
}
/*----------------------------------------------------------------------------*/
#ifndef CONFIG_PLATFORM_LPC_MRT_NO_DEINIT
static void tmrDeinit(void *object)
{
  struct Mrt * const timer = object;
  LPC_MRT_Type * const reg = timer->base.reg;

  reg->CHANNEL[timer->base.channel].INTVAL = INTVAL_LOAD;
  reg->CHANNEL[timer->base.channel].CTRL = 0;
  MrtBase->deinit(timer);
}
#endif
/*----------------------------------------------------------------------------*/
static void tmrEnable(void *object)
{
  struct Mrt * const timer = object;
  LPC_MRT_Type * const reg = timer->base.reg;

  reg->CHANNEL[timer->base.channel].INTVAL = timer->interval | INTVAL_LOAD;
}
/*----------------------------------------------------------------------------*/
static void tmrDisable(void *object)
{
  struct Mrt * const timer = object;
  LPC_MRT_Type * const reg = timer->base.reg;

  reg->CHANNEL[timer->base.channel].INTVAL = INTVAL_LOAD;
}
/*----------------------------------------------------------------------------*/
static void tmrSetAutostop(void *object, bool state)
{
  struct Mrt * const timer = object;
  LPC_MRT_Type * const reg = timer->base.reg;
  uint32_t control = reg->CHANNEL[timer->base.channel].CTRL;

  control &= ~CTRL_MODE_MASK;
  if (state)
    control |= CTRL_MODE_VALUE(MODE_ONE_SHOT_INTERRUPT);
  else
    control |= CTRL_MODE_VALUE(MODE_REPEAT_INTERRUPT);

  reg->CHANNEL[timer->base.channel].CTRL = control;
}
/*----------------------------------------------------------------------------*/
static void tmrSetCallback(void *object, void (*callback)(void *),
    void *argument)
{
  struct Mrt * const timer = object;
  LPC_MRT_Type * const reg = timer->base.reg;

  timer->callback = callback;
  timer->callbackArgument = argument;

  if (timer->callback != NULL)
    reg->CHANNEL[timer->base.channel].CTRL |= CTRL_INTEN;
  else
    reg->CHANNEL[timer->base.channel].CTRL &= ~CTRL_INTEN;
}
/*----------------------------------------------------------------------------*/
static uint32_t tmrGetFrequency(const void *object)
{
  const struct Mrt * const timer = object;
  return mrtGetClock(&timer->base);
}
/*----------------------------------------------------------------------------*/
static uint32_t tmrGetOverflow(const void *object)
{
  const struct Mrt * const timer = object;
  return timer->interval;
}
/*----------------------------------------------------------------------------*/
static void tmrSetOverflow(void *object, uint32_t overflow)
{
  struct Mrt * const timer = object;
  LPC_MRT_Type * const reg = timer->base.reg;

  assert(overflow <= TIMER_RESOLUTION);
  timer->interval = overflow ? overflow : TIMER_RESOLUTION;

  if (reg->CHANNEL[timer->base.channel].INTVAL)
    reg->CHANNEL[timer->base.channel].INTVAL = timer->interval | INTVAL_LOAD;
}
/*----------------------------------------------------------------------------*/
static uint32_t tmrGetValue(const void *object)
{
  const struct Mrt * const timer = object;
  const LPC_MRT_Type * const reg = timer->base.reg;

  return timer->interval - reg->CHANNEL[timer->base.channel].TIMER - 1;
}
/*----------------------------------------------------------------------------*/
static void tmrSetValue(void *object, [[maybe_unused]] uint32_t value)
{
  assert(value == 0);

  struct Mrt * const timer = object;
  LPC_MRT_Type * const reg = timer->base.reg;

  if (reg->CHANNEL[timer->base.channel].INTVAL)
    reg->CHANNEL[timer->base.channel].INTVAL = timer->interval | INTVAL_LOAD;
}
