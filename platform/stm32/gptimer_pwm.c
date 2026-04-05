/*
 * gptimer_pwm.c
 * Copyright (C) 2026 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/platform/stm32/gptimer_defs.h>
#include <halm/platform/stm32/gptimer_pwm.h>
#include <halm/pm.h>
#include <assert.h>
/*----------------------------------------------------------------------------*/
static void interruptHandler(void *);
static bool unitAllocateChannel(struct GpTimerPwmUnit *, uint8_t);

#ifdef CONFIG_PLATFORM_STM32_GPTIMER_PM
static void powerStateHandler(void *, enum PmState);
#endif
#ifndef CONFIG_PLATFORM_STM32_GPTIMER_NO_DEINIT
static void unitReleaseChannel(struct GpTimerPwmUnit *, uint8_t);
#endif
/*----------------------------------------------------------------------------*/
static enum Result unitInit(void *, const void *);
static void unitEnable(void *);
static void unitDisable(void *);
static void unitSetCallback(void *, void (*)(void *), void *);
static uint32_t unitGetFrequency(const void *);
static void unitSetFrequency(void *, uint32_t);
static uint32_t unitGetOverflow(const void *);
static void unitSetOverflow(void *, uint32_t);

#ifndef CONFIG_PLATFORM_STM32_GPTIMER_NO_DEINIT
static void unitDeinit(void *);
#else
#  define unitDeinit deletedDestructorTrap
#endif
/*----------------------------------------------------------------------------*/
static enum Result channelInit(void *, const void *);
static void channelEnable(void *);
static void channelDisable(void *);
static void channelSetDuration(void *, uint32_t);
static void channelSetEdges(void *, uint32_t, uint32_t);

#ifndef CONFIG_PLATFORM_STM32_GPTIMER_NO_DEINIT
static void channelDeinit(void *);
#else
#  define channelDeinit deletedDestructorTrap
#endif
/*----------------------------------------------------------------------------*/
const struct TimerClass * const GpTimerPwmUnit = &(const struct TimerClass){
    .size = sizeof(struct GpTimerPwmUnit),
    .init = unitInit,
    .deinit = unitDeinit,

    .enable = unitEnable,
    .disable = unitDisable,
    .setAutostop = NULL,
    .setCallback = unitSetCallback,
    .getFrequency = unitGetFrequency,
    .setFrequency = unitSetFrequency,
    .getOverflow = unitGetOverflow,
    .setOverflow = unitSetOverflow,
    .getValue = NULL,
    .setValue = NULL
};

const struct PwmClass * const GpTimerPwm = &(const struct PwmClass){
    .size = sizeof(struct GpTimerPwm),
    .init = channelInit,
    .deinit = channelDeinit,

    .enable = channelEnable,
    .disable = channelDisable,
    .setDuration = channelSetDuration,
    .setEdges = channelSetEdges
};
/*----------------------------------------------------------------------------*/
static void interruptHandler(void *object)
{
  struct GpTimerPwmUnit * const unit = object;
  STM_TIM_Type * const reg = unit->base.reg;

  if (reg->SR & SR_UIF)
  {
    /* Clear all pending interrupts */
    reg->SR = 0;

    unit->callback(unit->callbackArgument);
  }
}
/*----------------------------------------------------------------------------*/
#ifdef CONFIG_PLATFORM_STM32_GPTIMER_PM
static void powerStateHandler(void *object, enum PmState state)
{
  if (state == PM_ACTIVE)
  {
    struct GpTimerPwmUnit * const unit = object;
    gpTimerSetFrequency(&unit->base, unit->frequency);
  }
}
#endif
/*----------------------------------------------------------------------------*/
static bool unitAllocateChannel(struct GpTimerPwmUnit *unit, uint8_t channel)
{
  const uint8_t mask = 1 << channel;

  if (!(unit->matches & mask))
  {
    unit->matches |= mask;
    return true;
  }
  else
    return false;
}
/*----------------------------------------------------------------------------*/
#ifndef CONFIG_PLATFORM_STM32_GPTIMER_NO_DEINIT
static void unitReleaseChannel(struct GpTimerPwmUnit *unit, uint8_t channel)
{
  unit->matches &= ~(1 << channel);
}
#endif
/*----------------------------------------------------------------------------*/
static enum Result unitInit(void *object, const void *configBase)
{
  const struct GpTimerPwmUnitConfig * const config = configBase;
  assert(config != NULL);

  const struct GpTimerBaseConfig baseConfig = {
      .channel = config->channel
  };
  struct GpTimerPwmUnit * const unit = object;
  enum Result res;

  /* Call base class constructor */
  if ((res = GpTimerBase->init(unit, &baseConfig)) != E_OK)
    return res;
  /* Actual overflow value should be lower than a timer limit */
  assert(config->resolution > 0
      && config->resolution <= getMaxValue(unit->base.flags));

  unit->base.handler = interruptHandler;
  unit->callback = NULL;
  unit->frequency = config->frequency;
  unit->resolution = config->resolution;

  STM_TIM_Type * const reg = unit->base.reg;

  reg->CR1 = CR1_CKD(CKD_CK_INT) | CR1_CMS(CMS_EDGE_ALIGNED_MODE);
  reg->CR2 = CR2_MMS(MMS_UPDATE);
  reg->ARR = unit->resolution - 1;
  reg->CNT = 0;
  reg->DIER = 0;
  reg->CCMR[0] = 0;
  reg->CCMR[1] = 0;
  reg->CCER = 0;

  if (unit->base.flags & TIMER_FLAG_CONTROL)
    reg->BDTR |= BDTR_MOE;

  unit->matches = 0;
  gpTimerSetFrequency(&unit->base, unit->frequency);

#ifdef CONFIG_PLATFORM_STM32_GPTIMER_PM
  if ((res = pmRegister(powerStateHandler, unit)) != E_OK)
    return res;
#endif

  irqSetPriority(unit->base.irq, config->priority);
  irqEnable(unit->base.irq);

  /* Timer is left in a disabled state */
  return E_OK;
}
/*----------------------------------------------------------------------------*/
#ifndef CONFIG_PLATFORM_STM32_GPTIMER_NO_DEINIT
static void unitDeinit(void *object)
{
  struct GpTimerPwmUnit * const unit = object;
  STM_TIM_Type * const reg = unit->base.reg;

  irqDisable(unit->base.irq);
  reg->CR1 &= ~CR1_CEN;

#ifdef CONFIG_PLATFORM_STM32_GPTIMER_PM
  pmUnregister(unit);
#endif

  GpTimerBase->deinit(unit);
}
#endif
/*----------------------------------------------------------------------------*/
static void unitSetCallback(void *object, void (*callback)(void *),
    void *argument)
{
  struct GpTimerPwmUnit * const unit = object;
  STM_TIM_Type * const reg = unit->base.reg;

  unit->callbackArgument = argument;
  unit->callback = callback;

  if (unit->callback != NULL)
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
static void unitEnable(void *object)
{
  struct GpTimerPwmUnit * const unit = object;
  STM_TIM_Type * const reg = unit->base.reg;

  /* Clear pending interrupt flags */
  reg->SR = 0;
  /* Start the timer */
  reg->CR1 |= CR1_CEN;
}
/*----------------------------------------------------------------------------*/
static void unitDisable(void *object)
{
  struct GpTimerPwmUnit * const unit = object;
  STM_TIM_Type * const reg = unit->base.reg;

  reg->CR1 &= ~CR1_CEN;
}
/*----------------------------------------------------------------------------*/
static uint32_t unitGetFrequency(const void *object)
{
  const struct GpTimerPwmUnit * const unit = object;
  return unit->frequency;
}
/*----------------------------------------------------------------------------*/
static void unitSetFrequency(void *object, uint32_t frequency)
{
  struct GpTimerPwmUnit * const unit = object;

  unit->frequency = frequency;
  gpTimerSetFrequency(&unit->base, unit->frequency);
}
/*----------------------------------------------------------------------------*/
static uint32_t unitGetOverflow(const void *object)
{
  const struct GpTimerPwmUnit * const unit = object;
  return unit->resolution;
}
/*----------------------------------------------------------------------------*/
static void unitSetOverflow(void *object, uint32_t overflow)
{
  struct GpTimerPwmUnit * const unit = object;
  STM_TIM_Type * const reg = unit->base.reg;

  assert(overflow > 0 && overflow <= getMaxValue(unit->base.flags));
  unit->resolution = overflow;
  reg->ARR = unit->resolution - 1;
}
/*----------------------------------------------------------------------------*/
static enum Result channelInit(void *object, const void *configBase)
{
  const struct GpTimerPwmConfig * const config = configBase;
  assert(config != NULL);

  struct GpTimerPwm * const pwm = object;
  struct GpTimerPwmUnit * const unit = config->parent;

  const uint8_t channel = gpTimerGetOutputChannel(unit->base.channel,
      config->pin);

  /* Allocate match channel */
  if (!unitAllocateChannel(unit, channel))
    return E_BUSY;

  const unsigned int number = channel >> 1;
  const unsigned int part = channel >> 2;
  STM_TIM_Type * const reg = unit->base.reg;

  pwm->unit = unit;
  pwm->channel = channel;
  pwm->inversion = config->inversion;
  pwm->value = &reg->CCR[number];
  *pwm->value = 0;

  uint32_t ccer = reg->CCER & ~CCER_MASK(channel);
  uint32_t ccmr = reg->CCMR[part] & ~CCMR_MASK(number);

  ccer |= CCER_CCE(channel);
  if (pwm->inversion)
    ccer |= CCER_CCP(channel);

  ccmr |= CCMR_CCS(number, CCS_OUTPUT) | CCMR_OCPE(number)
      | CCMR_OCM(number, OCM_FORCE_LOW);

  reg->CCMR[part] = ccmr;
  reg->CCER = ccer;

  /* Initialize output pin after output level configuration */
  gpTimerConfigOutputPin(unit->base.channel, config->pin, pwm->inversion);

  return E_OK;
}
/*----------------------------------------------------------------------------*/
#ifndef CONFIG_PLATFORM_STM32_GPTIMER_NO_DEINIT
static void channelDeinit(void *object)
{
  struct GpTimerPwm * const pwm = object;

  channelDisable(pwm);
  unitReleaseChannel(pwm->unit, pwm->channel);
}
#endif
/*----------------------------------------------------------------------------*/
static void channelEnable(void *object)
{
  struct GpTimerPwm * const pwm = object;
  const unsigned int number = pwm->channel >> 1;
  const unsigned int part = pwm->channel >> 2;
  STM_TIM_Type * const reg = pwm->unit->base.reg;

  reg->CCMR[part] = (reg->CCMR[part] & ~CCMR_OCM_MASK(number))
      | CCMR_OCM(number, OCM_PWM_MODE_1);
}
/*----------------------------------------------------------------------------*/
static void channelDisable(void *object)
{
  struct GpTimerPwm * const pwm = object;
  const unsigned int number = pwm->channel >> 1;
  const unsigned int part = pwm->channel >> 2;
  STM_TIM_Type * const reg = pwm->unit->base.reg;

  reg->CCMR[part] = (reg->CCMR[part] & ~CCMR_OCM_MASK(number))
      | CCMR_OCM(number, OCM_FORCE_LOW);
}
/*----------------------------------------------------------------------------*/
static void channelSetDuration(void *object, uint32_t duration)
{
  struct GpTimerPwm * const pwm = object;

  assert(duration <= pwm->unit->resolution);
  *pwm->value = duration;
}
/*----------------------------------------------------------------------------*/
static void channelSetEdges(void *object, [[maybe_unused]] uint32_t leading,
    uint32_t trailing)
{
  struct GpTimerPwm * const pwm = object;

  assert(leading == 0); /* Leading edge time must be zero */
  assert(trailing <= pwm->unit->resolution);
  *pwm->value = trailing;
}
/*----------------------------------------------------------------------------*/
/**
 * Create single edge PWM channel.
 * @param unit Pointer to a GpTimerPwmUnit object.
 * @param pin Pin used as a signal output.
 * @param inversion Enable output inversion.
 * @return Pointer to a new Pwm object on success or zero on error.
 */
void *gpTimerPwmCreate(void *unit, PinNumber pin, bool inversion)
{
  const struct GpTimerPwmConfig channelConfig = {
      .parent = unit,
      .pin = pin,
      .inversion = inversion
  };

  return init(GpTimerPwm, &channelConfig);
}
