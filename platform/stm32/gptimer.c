/*
 * gptimer.c
 * Copyright (C) 2019 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/platform/platform_defs.h>
#include <halm/platform/stm32/gptimer.h>
#include <halm/platform/stm32/gptimer_defs.h>
#include <halm/pm.h>
#include <assert.h>
/*----------------------------------------------------------------------------*/
static void interruptHandler(void *);

#ifdef CONFIG_PLATFORM_STM32_GPTIMER_PM
static void powerStateHandler(void *, enum PmState);
#endif
/*----------------------------------------------------------------------------*/
static enum Result tmrInit(void *, const void *);
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

#ifndef CONFIG_PLATFORM_STM32_GPTIMER_NO_DEINIT
static void tmrDeinit(void *);
#else
#define tmrDeinit deletedDestructorTrap
#endif
/*----------------------------------------------------------------------------*/
const struct TimerClass * const GpTimer = &(const struct TimerClass){
    .size = sizeof(struct GpTimer),
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
static void interruptHandler(void *object)
{
  struct GpTimer * const timer = object;
  STM_TIM_Type * const reg = timer->base.reg;

  /* Clear all pending interrupts */
  reg->SR = 0;

  timer->callback(timer->callbackArgument);
}
/*----------------------------------------------------------------------------*/
#ifdef CONFIG_PLATFORM_STM32_GPTIMER_PM
static void powerStateHandler(void *object, enum PmState state)
{
  if (state == PM_ACTIVE)
  {
    struct GpTimer * const timer = object;
    gpTimerSetFrequency(&timer->base, timer->frequency);
  }
}
#endif
/*----------------------------------------------------------------------------*/
static enum Result tmrInit(void *object, const void *configBase)
{
  const struct GpTimerConfig * const config = configBase;
  assert(config != NULL);
  assert(config->event <= TIM_EVENT_CC4);

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
  timer->event = config->event;

  /* Initialize peripheral block */
  STM_TIM_Type * const reg = timer->base.reg;

  reg->CR1 = CR1_CKD(CKD_CK_INT) | CR1_CMS(CMS_EDGE_ALIGNED_MODE);
  reg->CR2 = CR2_MMS(MMS_UPDATE);
  reg->ARR = getMaxValue(timer->base.flags);

  if (config->event != TIM_EVENT_DISABLED)
  {
    if (config->event != TIM_EVENT_UPDATE)
    {
      const unsigned int number = config->event - TIM_EVENT_CC1;
      const unsigned int part = config->event > TIM_EVENT_CC2;

      reg->CCMR[part] = CCMR_OCM(number, OCM_TOGGLE);
      reg->CCER = CCER_CCE(number);
      reg->DIER = DIER_CCDE(number);
    }
    else
    {
      reg->DIER = DIER_UDE;
    }
  }

  timer->frequency = config->frequency;
  gpTimerSetFrequency(&timer->base, timer->frequency);

  if (timer->base.flags & TIMER_FLAG_INVERSE)
    reg->BDTR |= BDTR_MOE;

#ifdef CONFIG_PLATFORM_STM32_GPTIMER_PM
  if ((res = pmRegister(powerStateHandler, timer)) != E_OK)
    return res;
#endif

  irqSetPriority(timer->base.irq, config->priority);
  irqEnable(timer->base.irq);

  return E_OK;
}
/*----------------------------------------------------------------------------*/
#ifndef CONFIG_PLATFORM_STM32_GPTIMER_NO_DEINIT
static void tmrDeinit(void *object)
{
  struct GpTimer * const timer = object;
  STM_TIM_Type * const reg = timer->base.reg;

  irqDisable(timer->base.irq);
  reg->CR1 &= ~CR1_CEN;

#ifdef CONFIG_PLATFORM_STM32_GPTIMER_PM
  pmUnregister(timer);
#endif

  GpTimerBase->deinit(timer);
}
#endif
/*----------------------------------------------------------------------------*/
static void tmrEnable(void *object)
{
  struct GpTimer * const timer = object;
  STM_TIM_Type * const reg = timer->base.reg;

  /* Clear pending interrupt flags */
  reg->SR = 0;
  /* Start the timer */
  reg->CR1 |= CR1_CEN;
}
/*----------------------------------------------------------------------------*/
static void tmrDisable(void *object)
{
  struct GpTimer * const timer = object;
  STM_TIM_Type * const reg = timer->base.reg;

  reg->CR1 &= ~CR1_CEN;
}
/*----------------------------------------------------------------------------*/
static void tmrSetAutostop(void *object, bool state)
{
  struct GpTimer * const timer = object;
  STM_TIM_Type * const reg = timer->base.reg;

  if (state)
    reg->CR1 |= CR1_OPM;
  else
    reg->CR1 &= ~CR1_OPM;
}
/*----------------------------------------------------------------------------*/
static void tmrSetCallback(void *object, void (*callback)(void *),
    void *argument)
{
  struct GpTimer * const timer = object;
  STM_TIM_Type * const reg = timer->base.reg;

  timer->callbackArgument = argument;
  timer->callback = callback;

  if (timer->callback != NULL)
  {
    /* Clear pending interrupt flags */
    reg->SR = 0;
    /* Enable interrupt request generation on the update event */
    reg->DIER |= DIER_UIE;
  }
  else
  {
    /* Disable interrupt request generation */
    reg->DIER &= ~DIER_UIE;
  }
}
/*----------------------------------------------------------------------------*/
static uint32_t tmrGetFrequency(const void *object)
{
  const struct GpTimer * const timer = object;
  const STM_TIM_Type * const reg = timer->base.reg;
  const uint32_t apbClock = gpTimerGetClock(&timer->base);

  return apbClock / (reg->PSC + 1);
}
/*----------------------------------------------------------------------------*/
static void tmrSetFrequency(void *object, uint32_t frequency)
{
  struct GpTimer * const timer = object;

  timer->frequency = frequency;
  gpTimerSetFrequency(&timer->base, timer->frequency);
}
/*----------------------------------------------------------------------------*/
static uint32_t tmrGetOverflow(const void *object)
{
  const struct GpTimer * const timer = object;
  const STM_TIM_Type * const reg = timer->base.reg;

  return reg->ARR + 1;
}
/*----------------------------------------------------------------------------*/
static void tmrSetOverflow(void *object, uint32_t overflow)
{
  struct GpTimer * const timer = object;
  STM_TIM_Type * const reg = timer->base.reg;

  assert(overflow <= getMaxValue(timer->base.flags));

  if (timer->event > TIM_EVENT_UPDATE)
    reg->CCR[timer->event - TIM_EVENT_CC1] = overflow - 1;
  reg->ARR = overflow - 1;
}
/*----------------------------------------------------------------------------*/
static uint32_t tmrGetValue(const void *object)
{
  const struct GpTimer * const timer = object;
  const STM_TIM_Type * const reg = timer->base.reg;

  return reg->CNT;
}
/*----------------------------------------------------------------------------*/
static void tmrSetValue(void *object, uint32_t value)
{
  struct GpTimer * const timer = object;
  STM_TIM_Type * const reg = timer->base.reg;

  assert(value <= reg->ARR);
  reg->CNT = value;
}
