/*
 * pit.c
 * Copyright (C) 2024 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/platform/imxrt/pit.h>
#include <halm/platform/imxrt/pit_defs.h>
#include <assert.h>
/*----------------------------------------------------------------------------*/
static void interruptHandler(void *);
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

#ifndef CONFIG_PLATFORM_IMXRT_PIT_NO_DEINIT
static void tmrDeinit(void *);
#else
#  define tmrDeinit deletedDestructorTrap
#endif
/*----------------------------------------------------------------------------*/
const struct TimerClass * const Pit = &(const struct TimerClass){
    .size = sizeof(struct Pit),
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
static void interruptHandler(void *object)
{
  struct Pit * const timer = object;
  IMX_PIT_Type * const reg = timer->base.reg;

  /* Clear pending interrupt flag */
  reg->CHANNEL[timer->base.channel].TFLG = TFLG_TIF;

  timer->callback(timer->callbackArgument);
}
/*----------------------------------------------------------------------------*/
static enum Result tmrInit(void *object, const void *configBase)
{
  const struct PitConfig * const config = configBase;
  assert(config != NULL);

  const struct PitBaseConfig baseConfig = {
      .channel = config->channel
  };
  struct Pit * const timer = object;
  enum Result res;

  /* Call base class constructor */
  if ((res = PitBase->init(timer, &baseConfig)) != E_OK)
    return res;

  timer->base.handler = interruptHandler;
  timer->callback = NULL;

  IMX_PIT_Type * const reg = timer->base.reg;

  reg->CHANNEL[timer->base.channel].TCTRL = 0;
  reg->CHANNEL[timer->base.channel].LDVAL = UINT32_MAX;

  irqSetPriority(timer->base.irq, config->priority);
  return E_OK;
}
/*----------------------------------------------------------------------------*/
#ifndef CONFIG_PLATFORM_IMXRT_PIT_NO_DEINIT
static void tmrDeinit(void *object)
{
  struct Pit * const timer = object;
  IMX_PIT_Type * const reg = timer->base.reg;

  reg->CHANNEL[timer->base.channel].TCTRL = 0;

  PitBase->deinit(timer);
}
#endif
/*----------------------------------------------------------------------------*/
static void tmrEnable(void *object)
{
  struct Pit * const timer = object;
  IMX_PIT_Type * const reg = timer->base.reg;

  reg->CHANNEL[timer->base.channel].TCTRL |= TCTRL_TEN;
}
/*----------------------------------------------------------------------------*/
static void tmrDisable(void *object)
{
  struct Pit * const timer = object;
  IMX_PIT_Type * const reg = timer->base.reg;

  reg->CHANNEL[timer->base.channel].TCTRL &= ~TCTRL_TEN;
}
/*----------------------------------------------------------------------------*/
static void tmrSetCallback(void *object, void (*callback)(void *),
    void *argument)
{
  struct Pit * const timer = object;
  IMX_PIT_Type * const reg = timer->base.reg;

  timer->callback = callback;
  timer->callbackArgument = argument;

  if (timer->callback != NULL)
    reg->CHANNEL[timer->base.channel].TCTRL |= TCTRL_TIE;
  else
    reg->CHANNEL[timer->base.channel].TCTRL &= ~TCTRL_TIE;
}
/*----------------------------------------------------------------------------*/
static uint32_t tmrGetFrequency(const void *object)
{
  return pitGetClock(object);
}
/*----------------------------------------------------------------------------*/
static uint32_t tmrGetOverflow(const void *object)
{
  const struct Pit * const timer = object;
  const IMX_PIT_Type * const reg = timer->base.reg;

  return reg->CHANNEL[timer->base.channel].LDVAL + 1;
}
/*----------------------------------------------------------------------------*/
static void tmrSetOverflow(void *object, uint32_t overflow)
{
  struct Pit * const timer = object;
  IMX_PIT_Type * const reg = timer->base.reg;

  reg->CHANNEL[timer->base.channel].LDVAL = overflow - 1;
}
/*----------------------------------------------------------------------------*/
static uint32_t tmrGetValue(const void *object)
{
  const struct Pit * const timer = object;
  const IMX_PIT_Type * const reg = timer->base.reg;

  return reg->CHANNEL[timer->base.channel].CVAL;
}
/*----------------------------------------------------------------------------*/
static void tmrSetValue(void *object, uint32_t value)
{
  assert(value == 0);

  struct Pit * const timer = object;
  IMX_PIT_Type * const reg = timer->base.reg;

  if (reg->CHANNEL[timer->base.channel].TCTRL & TCTRL_TIE)
  {
    /* Clear the timer by disabling and then enabling it back */
    reg->CHANNEL[timer->base.channel].TCTRL &= ~TCTRL_TIE;
    reg->CHANNEL[timer->base.channel].TCTRL |= TCTRL_TIE;
  }
}
