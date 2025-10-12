/*
 * wkt.c
 * Copyright (C) 2025 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/platform/lpc/wkt.h>
#include <halm/platform/lpc/wkt_defs.h>
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

#ifndef CONFIG_PLATFORM_LPC_WKT_NO_DEINIT
static void tmrDeinit(void *);
#else
#  define tmrDeinit deletedDestructorTrap
#endif
/*----------------------------------------------------------------------------*/
const struct TimerClass * const Wkt = &(const struct TimerClass){
    .size = sizeof(struct Wkt),
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
  struct Wkt * const timer = object;
  LPC_WKT_Type * const reg = timer->base.reg;

  reg->CTRL |= CTRL_ALARMFLAG;

  if (timer->restart)
    reg->COUNT = timer->overflow;

  if (timer->callback != NULL)
    timer->callback(timer->callbackArgument);
}
/*----------------------------------------------------------------------------*/
static enum Result tmrInit(void *object, const void *configBase)
{
  const struct WktConfig * const config = configBase;
  assert(config != NULL);

  const struct WktBaseConfig baseConfig = {
      .pin = config->pin,
      .source = config->source
  };
  struct Wkt * const timer = object;
  enum Result res;

  /* Call base class constructor */
  if ((res = WktBase->init(timer, &baseConfig)) != E_OK)
    return res;

  timer->base.handler = interruptHandler;
  timer->callback = NULL;
  timer->overflow = TIMER_RESOLUTION;
  timer->restart = true;

  LPC_WKT_Type * const reg = timer->base.reg;

  timer->fired = (reg->CTRL & CTRL_ALARMFLAG) != 0;
  reg->CTRL |= CTRL_ALARMFLAG | CTRL_CLEARCTR;

  irqSetPriority(timer->base.irq, config->priority);
  irqEnable(timer->base.irq);

  return E_OK;
}
/*----------------------------------------------------------------------------*/
#ifndef CONFIG_PLATFORM_LPC_WKT_NO_DEINIT
static void tmrDeinit(void *object)
{
  struct Wkt * const timer = object;
  LPC_WKT_Type * const reg = timer->base.reg;

  irqDisable(timer->base.irq);
  reg->CTRL |= CTRL_CLEARCTR;

  WktBase->deinit(timer);
}
#endif
/*----------------------------------------------------------------------------*/
static void tmrEnable(void *object)
{
  struct Wkt * const timer = object;
  LPC_WKT_Type * const reg = timer->base.reg;

  reg->COUNT = timer->overflow;
}
/*----------------------------------------------------------------------------*/
static void tmrDisable(void *object)
{
  struct Wkt * const timer = object;
  LPC_WKT_Type * const reg = timer->base.reg;

  reg->CTRL |= CTRL_CLEARCTR;
}
/*----------------------------------------------------------------------------*/
static void tmrSetAutostop(void *object, bool state)
{
  struct Wkt * const timer = object;
  timer->restart = !state;
}
/*----------------------------------------------------------------------------*/
static void tmrSetCallback(void *object, void (*callback)(void *),
    void *argument)
{
  struct Wkt * const timer = object;

  timer->callback = callback;
  timer->callbackArgument = argument;
}
/*----------------------------------------------------------------------------*/
static uint32_t tmrGetFrequency(const void *object)
{
  const struct Wkt * const timer = object;
  return wktGetClock(&timer->base);
}
/*----------------------------------------------------------------------------*/
static uint32_t tmrGetOverflow(const void *object)
{
  const struct Wkt * const timer = object;
  return timer->overflow;
}
/*----------------------------------------------------------------------------*/
static void tmrSetOverflow(void *object, uint32_t overflow)
{
  struct Wkt * const timer = object;

  assert(overflow <= TIMER_RESOLUTION);
  timer->overflow = overflow ? overflow : TIMER_RESOLUTION;
}
/*----------------------------------------------------------------------------*/
static uint32_t tmrGetValue(const void *object)
{
  const struct Wkt * const timer = object;
  const LPC_WKT_Type * const reg = timer->base.reg;

  return timer->overflow - reg->COUNT;
}
/*----------------------------------------------------------------------------*/
static void tmrSetValue(void *object, [[maybe_unused]] uint32_t value)
{
  assert(value == 0);

  struct Wkt * const timer = object;
  LPC_WKT_Type * const reg = timer->base.reg;

  reg->CTRL |= CTRL_CLEARCTR;
}
/*----------------------------------------------------------------------------*/
bool wktWakeupOccurred(const struct Wkt *timer)
{
  return timer->fired;
}
