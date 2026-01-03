/*
 * gptimer.c
 * Copyright (C) 2026 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/platform/bouffalo/gptimer.h>
#include <halm/platform/bouffalo/gptimer_defs.h>
#include <halm/platform/platform_defs.h>
#include <halm/pm.h>
#include <assert.h>
/*----------------------------------------------------------------------------*/
#define MATCH_EVENT 0
/*----------------------------------------------------------------------------*/
static void interruptHandler(void *);

#ifdef CONFIG_PLATFORM_BOUFFALO_GPTIMER_PM
static void powerStateHandler(void *, enum PmState);
#endif
/*----------------------------------------------------------------------------*/
static enum Result tmrInit(void *, const void *);
static void tmrEnable(void *);
static void tmrDisable(void *);
static void tmrSetCallback(void *, void (*)(void *), void *);
static uint32_t tmrGetFrequency(const void *);
static void tmrSetFrequency(void *, uint32_t);
static uint32_t tmrGetOverflow(const void *);
static void tmrSetOverflow(void *, uint32_t);
static uint32_t tmrGetValue(const void *);
static void tmrSetValue(void *, uint32_t);

#ifndef CONFIG_PLATFORM_BOUFFALO_GPTIMER_NO_DEINIT
static void tmrDeinit(void *);
#else
#  define tmrDeinit deletedDestructorTrap
#endif
/*----------------------------------------------------------------------------*/
const struct TimerClass * const GpTimer = &(const struct TimerClass){
    .size = sizeof(struct GpTimer),
    .init = tmrInit,
    .deinit = tmrDeinit,

    .enable = tmrEnable,
    .disable = tmrDisable,
    .setAutostop = NULL,
    .setCallback = tmrSetCallback,
    .getFrequency = tmrGetFrequency,
    .setFrequency = tmrSetFrequency,
    .getOverflow = tmrGetOverflow,
    .setOverflow = tmrSetOverflow,
    .getValue = tmrGetValue,
    .setValue = tmrSetValue
};
/*----------------------------------------------------------------------------*/
static void interruptHandler(void *object)
{
  struct GpTimer * const timer = object;

  BL_TIMER->TICR[timer->base.channel] = TICR_MASK;
  timer->callback(timer->callbackArgument);
}
/*----------------------------------------------------------------------------*/
static void setTimerFrequency(struct GpTimer *timer, uint32_t frequency)
{
  uint32_t prescaler;

  if (frequency)
  {
    prescaler = gpTimerGetClock(&timer->base) / frequency - 1;
    assert(prescaler <= TCDR_CDR_MAX);
  }
  else
    prescaler = 0;

  BL_TIMER->TCDR = (BL_TIMER->TCDR & ~TCDR_CDR_MASK(timer->base.channel))
      | TCDR_CDR(timer->base.channel, prescaler);
}
/*----------------------------------------------------------------------------*/
#ifdef CONFIG_PLATFORM_BOUFFALO_GPTIMER_PM
static void powerStateHandler(void *object, enum PmState state)
{
  if (state == PM_ACTIVE)
  {
    struct GpTimer * const timer = object;
    setTimerFrequency(timer, timer->frequency);
  }
}
#endif
/*----------------------------------------------------------------------------*/
static enum Result tmrInit(void *object, const void *configBase)
{
  const struct GpTimerConfig * const config = configBase;
  assert(config != NULL);

  const struct GpTimerBaseConfig baseConfig = {
      .channel = config->channel
  };
  struct GpTimer * const timer = object;
  enum Result res;

  /* Call base class constructor */
  if ((res = GpTimerBase->init(timer, &baseConfig)) != E_OK)
    return res;

  timer->base.handler = interruptHandler;
  timer->callback = NULL;

  /* Initialize peripheral block */
  const uint8_t channel = timer->base.channel;

  BL_TIMER->TCER &= ~TCER_ENABLE(timer->base.channel);
  BL_TIMER->TIER[channel] = 0;
  BL_TIMER->TICR[channel] = TICR_MASK;
  BL_TIMER->TPLVR[channel] = 0;
  BL_TIMER->TPLCR[channel] = TPLCR_PLCR(PLCR_MATCH0);
  BL_TIMER->TMR[channel].MAT[MATCH_EVENT] = UINT32_MAX;

  setTimerFrequency(timer, config->frequency);
  timer->frequency = config->frequency;

#ifdef CONFIG_PLATFORM_BOUFFALO_GPTIMER_PM
  if ((res = pmRegister(powerStateHandler, timer)) != E_OK)
    return res;
#endif

  irqSetPriority(timer->base.irq, config->priority);
  irqEnable(timer->base.irq);

  return E_OK;
}
/*----------------------------------------------------------------------------*/
#ifndef CONFIG_PLATFORM_BOUFFALO_GPTIMER_NO_DEINIT
static void tmrDeinit(void *object)
{
  struct GpTimer * const timer = object;

  irqDisable(timer->base.irq);
  BL_TIMER->TCER &= ~TCER_ENABLE(timer->base.channel);

#ifdef CONFIG_PLATFORM_BOUFFALO_GPTIMER_PM
  pmUnregister(timer);
#endif

  GpTimerBase->deinit(timer);
}
#endif
/*----------------------------------------------------------------------------*/
static void tmrEnable(void *object)
{
  struct GpTimer * const timer = object;

  BL_TIMER->TICR[timer->base.channel] = TICR_MASK;
  BL_TIMER->TCER |= TCER_ENABLE(timer->base.channel);
}
/*----------------------------------------------------------------------------*/
static void tmrDisable(void *object)
{
  struct GpTimer * const timer = object;
  BL_TIMER->TCER &= ~TCER_ENABLE(timer->base.channel);
}
/*----------------------------------------------------------------------------*/
static void tmrSetCallback(void *object, void (*callback)(void *),
    void *argument)
{
  struct GpTimer * const timer = object;
  const uint8_t channel = timer->base.channel;

  timer->callbackArgument = argument;
  timer->callback = callback;

  if (timer->callback != NULL)
  {
    /* Clear pending interrupt flags */
    BL_TIMER->TICR[channel] = TICR_MASK;
    /* Enable interrupt request on timer match event */
    BL_TIMER->TIER[channel] = TIER_ENABLE(MATCH_EVENT);
  }
  else
  {
    /* Disable further interrupt requests */
    BL_TIMER->TIER[channel] = 0;
  }
}
/*----------------------------------------------------------------------------*/
static uint32_t tmrGetFrequency(const void *object)
{
  const struct GpTimer * const timer = object;
  const uint32_t prescaler =
      TCDR_CDR_VALUE(timer->base.channel, BL_TIMER->TCDR) + 1;

  return gpTimerGetClock(&timer->base) / prescaler;
}
/*----------------------------------------------------------------------------*/
static void tmrSetFrequency(void *object, uint32_t frequency)
{
  struct GpTimer * const timer = object;

  timer->frequency = frequency;
  setTimerFrequency(timer, timer->frequency);
}
/*----------------------------------------------------------------------------*/
static uint32_t tmrGetOverflow(const void *object)
{
  const struct GpTimer * const timer = object;
  return BL_TIMER->TMR[timer->base.channel].MAT[MATCH_EVENT] + 1;
}
/*----------------------------------------------------------------------------*/
static void tmrSetOverflow(void *object, uint32_t overflow)
{
  struct GpTimer * const timer = object;
  BL_TIMER->TMR[timer->base.channel].MAT[MATCH_EVENT] = overflow - 1;
}
/*----------------------------------------------------------------------------*/
static uint32_t tmrGetValue(const void *object)
{
  const struct GpTimer * const timer = object;
  return BL_TIMER->TCVSYN[timer->base.channel];
}
/*----------------------------------------------------------------------------*/
static void tmrSetValue(void *object, uint32_t value)
{
  struct GpTimer * const timer = object;

  assert(value <= BL_TIMER->TMR[timer->base.channel].MAT[MATCH_EVENT]);
  BL_TIMER->TCVSYN[timer->base.channel] = value;
}
