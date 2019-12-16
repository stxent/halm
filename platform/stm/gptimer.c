/*
 * gptimer.c
 * Copyright (C) 2019 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <assert.h>
#include <halm/platform/platform_defs.h>
#include <halm/platform/stm/gptimer.h>
#include <halm/platform/stm/gptimer_defs.h>
#include <halm/pm.h>
/*----------------------------------------------------------------------------*/
static inline uint32_t getMaxValue(const struct GpTimer *);
static void interruptHandler(void *);
static void setTimerFrequency(struct GpTimer *, uint32_t);

#ifdef CONFIG_PLATFORM_STM_GPTIMER_PM
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

#ifndef CONFIG_PLATFORM_STM_GPTIMER_NO_DEINIT
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
    .setCallback = tmrSetCallback,
    .getFrequency = tmrGetFrequency,
    .setFrequency = tmrSetFrequency,
    .getOverflow = tmrGetOverflow,
    .setOverflow = tmrSetOverflow,
    .getValue = tmrGetValue,
    .setValue = tmrSetValue
};
/*----------------------------------------------------------------------------*/
static inline uint32_t getMaxValue(const struct GpTimer *timer)
{
  return MASK(timer->base.resolution);
}
/*----------------------------------------------------------------------------*/
static void interruptHandler(void *object)
{
  struct GpTimer * const timer = object;
  STM_TIM_Type * const reg = timer->base.reg;

  /* Clear all pending interrupts */
  reg->SR = ~SR_CCIF_MASK;

  timer->callback(timer->callbackArgument);
}
/*----------------------------------------------------------------------------*/
#ifdef CONFIG_PLATFORM_STM_GPTIMER_PM
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
static void setTimerFrequency(struct GpTimer *timer, uint32_t frequency)
{
  STM_TIM_Type * const reg = timer->base.reg;
  uint32_t divisor;

  if (frequency)
  {
    const uint32_t apbFrequency = gpTimerGetClock(&timer->base);
    assert(frequency <= apbFrequency);

    divisor = apbFrequency / frequency - 1;
    assert(divisor <= MASK(timer->base.resolution));
  }
  else
    divisor = 0;

  reg->PSC = divisor;
  reg->EGR = EGR_UG;
}
/*----------------------------------------------------------------------------*/
static enum Result tmrInit(void *object, const void *configBase)
{
  const struct GpTimerConfig * const config = configBase;
  assert(config);

  const struct GpTimerBaseConfig baseConfig = {
      .channel = config->channel
  };
  struct GpTimer * const timer = object;
  enum Result res;

  assert(config->event <= 4);

  /* Call base class constructor */
  if ((res = GpTimerBase->init(timer, &baseConfig)) != E_OK)
    return res;

  timer->base.handler = interruptHandler;
  timer->callback = 0;
  timer->event = config->event ? config->event - 1 : 0;

  /* Initialize peripheral block */
  STM_TIM_Type * const reg = timer->base.reg;

  reg->CR1 = CR1_CKD(CKD_CK_INT) | CR1_CMS(CMS_EDGE_ALIGNED_MODE);
  reg->ARR = getMaxValue(timer);
  reg->DIER = 0;

  timer->frequency = config->frequency;
  setTimerFrequency(timer, timer->frequency);

//  reg->CR2 &= ~CR2_CCPC; // TODO Advanced timers

#ifdef CONFIG_PLATFORM_STM_GPTIMER_PM
  if ((res = pmRegister(powerStateHandler, timer)) != E_OK)
    return res;
#endif

  irqSetPriority(timer->base.irq, config->priority);
  irqEnable(timer->base.irq);

  return E_OK;
}
/*----------------------------------------------------------------------------*/
#ifndef CONFIG_PLATFORM_STM_GPTIMER_NO_DEINIT
static void tmrDeinit(void *object)
{
  struct GpTimer * const timer = object;
  STM_TIM_Type * const reg = timer->base.reg;

  irqDisable(timer->base.irq);
  reg->CR1 &= ~CR1_CEN;

#ifdef CONFIG_PLATFORM_STM_GPTIMER_PM
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

  /* Enable DMA request */
  if (timer->event)
    reg->DIER |= DIER_CCDE(timer->event);

  /* Start the timer */
  reg->CR1 |= CR1_CEN;
}
/*----------------------------------------------------------------------------*/
static void tmrDisable(void *object)
{
  struct GpTimer * const timer = object;
  STM_TIM_Type * const reg = timer->base.reg;

  reg->CR1 &= ~CR1_CEN;
  reg->DIER &= ~(DIER_CCIE_MASK | DIER_CCDE_MASK);
}
/*----------------------------------------------------------------------------*/
static void tmrSetCallback(void *object, void (*callback)(void *),
    void *argument)
{
  struct GpTimer * const timer = object;
  STM_TIM_Type * const reg = timer->base.reg;

  timer->callbackArgument = argument;
  timer->callback = callback;

  if (callback)
  {
    /* Clear pending interrupt flags */
    reg->SR = 0;
    /* Enable generation of an interrupt request */
    reg->DIER |= DIER_CCIE(timer->event);
  }
  else
  {
    /* Disable interrupt request generation */
    reg->DIER &= ~DIER_CCIE_MASK;
  }
}
/*----------------------------------------------------------------------------*/
static uint32_t tmrGetFrequency(const void *object)
{
  const struct GpTimer * const timer = object;
  const STM_TIM_Type * const reg = timer->base.reg;
  const uint32_t baseClock = gpTimerGetClock(&timer->base);

  return baseClock / (reg->PSC + 1);
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
  const STM_TIM_Type * const reg = timer->base.reg;

  return (reg->ARR + 1) & getMaxValue(timer);
}
/*----------------------------------------------------------------------------*/
static void tmrSetOverflow(void *object, uint32_t overflow)
{
  struct GpTimer * const timer = object;
  STM_TIM_Type * const reg = timer->base.reg;

  assert(overflow <= getMaxValue(timer));

  reg->CCR[timer->event] = overflow - 1;
  reg->ARR = overflow - 1;
  reg->EGR = EGR_UG;
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
