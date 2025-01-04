/*
 * gptimer_pwm.c
 * Copyright (C) 2013 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/platform/lpc/gptimer_pwm.h>
#include <halm/platform/lpc/gptimer_pwm_defs.h>
#include <halm/pm.h>
#include <assert.h>
/*----------------------------------------------------------------------------*/
static void interruptHandler(void *);
static bool unitAllocateChannel(struct GpTimerPwmUnit *, uint8_t);
static void unitUpdateResolution(struct GpTimerPwmUnit *, uint8_t);

#ifdef CONFIG_PLATFORM_LPC_GPTIMER_PM
static void powerStateHandler(void *, enum PmState);
#endif
#ifndef CONFIG_PLATFORM_LPC_GPTIMER_NO_DEINIT
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

#ifndef CONFIG_PLATFORM_LPC_GPTIMER_NO_DEINIT
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

#ifndef CONFIG_PLATFORM_LPC_GPTIMER_NO_DEINIT
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
  LPC_TIMER_Type * const reg = unit->base.reg;

  /* Clear all pending interrupts */
  reg->IR = IR_MATCH_MASK | IR_CAPTURE_MASK;

  unit->callback(unit->callbackArgument);
}
/*----------------------------------------------------------------------------*/
#ifdef CONFIG_PLATFORM_LPC_GPTIMER_PM
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
  const int freeChannel = gpTimerAllocateChannel(unit->matches | mask);

  if (freeChannel != -1 && !(unit->matches & mask))
  {
    unit->matches |= mask;
    unitUpdateResolution(unit, freeChannel);
    return true;
  }
  else
    return false;
}
/*----------------------------------------------------------------------------*/
#ifndef CONFIG_PLATFORM_LPC_GPTIMER_NO_DEINIT
static void unitReleaseChannel(struct GpTimerPwmUnit *unit, uint8_t channel)
{
  unit->matches &= ~(1 << channel);
}
#endif
/*----------------------------------------------------------------------------*/
static void unitUpdateResolution(struct GpTimerPwmUnit *unit, uint8_t channel)
{
  LPC_TIMER_Type * const reg = unit->base.reg;

  /* Put the timer into a reset state to clear prescaler and counter */
  reg->TCR |= TCR_CRES;

  /* Disable previous match channel */
  reg->MCR &= ~MCR_RESET(unit->limiter);

  /* Initialize new match channel and enable it */
  unit->limiter = channel;
  reg->MR[unit->limiter] = unit->resolution - 1;
  reg->MCR |= MCR_RESET(unit->limiter);

  /* Clear reset bit and enable counting */
  reg->TCR &= ~TCR_CRES;
}
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
      && config->resolution <= gpTimerGetMaxValue(&unit->base));

  unit->base.handler = interruptHandler;
  unit->callback = NULL;
  unit->frequency = config->frequency;
  unit->resolution = config->resolution;

  LPC_TIMER_Type * const reg = unit->base.reg;

  reg->TCR = TCR_CRES;
  reg->IR = reg->IR; /* Clear pending interrupts */
  reg->CTCR = 0;
  reg->CCR = 0;
  reg->EMR = 0;
  reg->PWMC = 0; /* Register is available only on specific parts */

  /* Configure timings */
  gpTimerSetFrequency(&unit->base, unit->frequency);

  unit->matches = 0;
  unit->limiter = gpTimerAllocateChannel(unit->matches);
  reg->MR[unit->limiter] = unit->resolution - 1;
  reg->MCR = MCR_RESET(unit->limiter);

#ifdef CONFIG_PLATFORM_LPC_GPTIMER_PM
  if ((res = pmRegister(powerStateHandler, unit)) != E_OK)
    return res;
#endif

  irqSetPriority(unit->base.irq, config->priority);
  irqEnable(unit->base.irq);

  /* Timer is left in a disabled state */
  return E_OK;
}
/*----------------------------------------------------------------------------*/
#ifndef CONFIG_PLATFORM_LPC_GPTIMER_NO_DEINIT
static void unitDeinit(void *object)
{
  struct GpTimerPwmUnit * const unit = object;
  LPC_TIMER_Type * const reg = unit->base.reg;

  irqDisable(unit->base.irq);
  reg->TCR = 0;

#ifdef CONFIG_PLATFORM_LPC_GPTIMER_PM
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
  LPC_TIMER_Type * const reg = unit->base.reg;
  const uint32_t mask = MCR_INTERRUPT(unit->limiter);

  unit->callbackArgument = argument;
  unit->callback = callback;

  if (unit->callback != NULL)
  {
    reg->IR = IR_MATCH_MASK | IR_CAPTURE_MASK;
    reg->MCR |= mask;
  }
  else
    reg->MCR &= ~mask;
}
/*----------------------------------------------------------------------------*/
static void unitEnable(void *object)
{
  struct GpTimerPwmUnit * const unit = object;
  LPC_TIMER_Type * const reg = unit->base.reg;

  /* Clear pending interrupt flags */
  reg->IR = IR_MATCH_MASK | IR_CAPTURE_MASK;
  /* Start the timer */
  reg->TCR = TCR_CEN;
}
/*----------------------------------------------------------------------------*/
static void unitDisable(void *object)
{
  struct GpTimerPwmUnit * const unit = object;
  LPC_TIMER_Type * const reg = unit->base.reg;

  /* Stop the timer */
  reg->TCR = 0;
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
  LPC_TIMER_Type * const reg = unit->base.reg;

  assert(overflow > 0 && overflow <= gpTimerGetMaxValue(&unit->base));
  unit->resolution = overflow;
  reg->MR[unit->limiter] = unit->resolution - 1;
}
/*----------------------------------------------------------------------------*/
static enum Result channelInit(void *object, const void *configBase)
{
  const struct GpTimerPwmConfig * const config = configBase;
  assert(config != NULL);

  struct GpTimerPwm * const pwm = object;
  struct GpTimerPwmUnit * const unit = config->parent;

  const uint8_t channel = gpTimerGetMatchChannel(unit->base.channel,
      config->pin);

  /* Allocate match channel */
  if (!unitAllocateChannel(unit, channel))
    return E_BUSY;

  const uint32_t resolution = unit->resolution;
  LPC_TIMER_Type * const reg = unit->base.reg;

  pwm->unit = unit;
  pwm->value = &reg->MR[channel];
  pwm->channel = channel;
  pwm->inversion = config->inversion;

  /* Update match state to avoid undefined output level */
  if (pwm->inversion)
  {
    reg->EMR |= EMR_EXTERNAL_MATCH(channel);
    *pwm->value = 0;
  }
  else
  {
    reg->EMR &= ~EMR_EXTERNAL_MATCH(channel);
    *pwm->value = resolution + 1;
  }

  /* Initialize output pin after match state configuration */
  gpTimerConfigMatchPin(unit->base.channel, config->pin);

  return E_OK;
}
/*----------------------------------------------------------------------------*/
#ifndef CONFIG_PLATFORM_LPC_GPTIMER_NO_DEINIT
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
  LPC_TIMER_Type * const reg = pwm->unit->base.reg;

  reg->PWMC |= PWMC_ENABLE(pwm->channel);
}
/*----------------------------------------------------------------------------*/
static void channelDisable(void *object)
{
  struct GpTimerPwm * const pwm = object;
  LPC_TIMER_Type * const reg = pwm->unit->base.reg;
  const uint8_t channel = pwm->channel;

  reg->PWMC &= ~PWMC_ENABLE(channel);

  /* Overwrite match output state to avoid undefined output level */
  if (pwm->inversion)
    reg->EMR |= EMR_EXTERNAL_MATCH(channel);
  else
    reg->EMR &= ~EMR_EXTERNAL_MATCH(channel);
}
/*----------------------------------------------------------------------------*/
static void channelSetDuration(void *object, uint32_t duration)
{
  struct GpTimerPwm * const pwm = object;
  const uint32_t resolution = pwm->unit->resolution;

  if (duration)
  {
    if (duration > resolution)
      duration = resolution;

    if (!pwm->inversion)
    {
      /* The output itself is inverted */
      duration = resolution - duration;
    }
  }
  else
  {
    if (!pwm->inversion)
    {
      /*
       * If a match register is set to a value greater or equal to
       * the resolution, then the output stays low during all cycle.
       */
      duration = resolution + 1;
    }
    else
    {
      /*
       * If a match register is set to zero, than the output goes high
       * and will stay in this state indefinitely.
       */
      duration = 0;
    }
  }

  *pwm->value = duration;
}
/*----------------------------------------------------------------------------*/
static void channelSetEdges(void *object, [[maybe_unused]] uint32_t leading,
    uint32_t trailing)
{
  assert(leading == 0); /* Leading edge time must be zero */
  channelSetDuration(object, trailing);
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
