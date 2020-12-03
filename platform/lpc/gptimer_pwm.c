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

#ifndef CONFIG_PLATFORM_LPC_GPTIMER_NO_DEINIT
static void unitDeinit(void *);
#else
#define unitDeinit deletedDestructorTrap
#endif
/*----------------------------------------------------------------------------*/
static enum Result channelInit(void *, const void *);
static void channelEnable(void *);
static void channelDisable(void *);
static uint32_t channelGetResolution(const void *);
static void channelSetDuration(void *, uint32_t);
static void channelSetEdges(void *, uint32_t, uint32_t);
static void channelSetFrequency(void *, uint32_t);

#ifndef CONFIG_PLATFORM_LPC_GPTIMER_NO_DEINIT
static void channelDeinit(void *);
#else
#define channelDeinit deletedDestructorTrap
#endif
/*----------------------------------------------------------------------------*/
const struct EntityClass * const GpTimerPwmUnit = &(const struct EntityClass){
    .size = sizeof(struct GpTimerPwmUnit),
    .init = unitInit,
    .deinit = unitDeinit
};

const struct PwmClass * const GpTimerPwm = &(const struct PwmClass){
    .size = sizeof(struct GpTimerPwm),
    .init = channelInit,
    .deinit = channelDeinit,

    .enable = channelEnable,
    .disable = channelDisable,
    .getResolution = channelGetResolution,
    .setDuration = channelSetDuration,
    .setEdges = channelSetEdges,
    .setFrequency = channelSetFrequency
};
/*----------------------------------------------------------------------------*/
#ifdef CONFIG_PLATFORM_LPC_GPTIMER_PM
static void powerStateHandler(void *object, enum PmState state)
{
  if (state == PM_ACTIVE)
  {
    struct GpTimerPwmUnit * const unit = object;
    gpTimerSetFrequency(&unit->base, unit->frequency * unit->resolution);
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
  assert(config);
  assert(config->resolution >= 2);

  const struct GpTimerBaseConfig baseConfig = {
      .channel = config->channel
  };
  struct GpTimerPwmUnit * const unit = object;
  enum Result res;

  /* Call base class constructor */
  if ((res = GpTimerBase->init(unit, &baseConfig)) != E_OK)
    return res;
  assert(config->resolution < MASK(unit->base.resolution));

  LPC_TIMER_Type * const reg = unit->base.reg;

  reg->TCR = TCR_CRES;

  reg->IR = reg->IR; /* Clear pending interrupts */
  reg->CTCR = 0;
  reg->CCR = 0;
  reg->EMR = 0;
  reg->PWMC = 0; /* Register is available only on specific parts */

  /* Configure timings */
  unit->frequency = config->frequency;
  unit->resolution = config->resolution;
  gpTimerSetFrequency(&unit->base, unit->frequency * unit->resolution);

  unit->matches = 0;
  unit->limiter = gpTimerAllocateChannel(unit->matches);
  reg->MR[unit->limiter] = unit->resolution - 1;
  reg->MCR = MCR_RESET(unit->limiter);

#ifdef CONFIG_PLATFORM_LPC_GPTIMER_PM
  if ((res = pmRegister(powerStateHandler, unit)) != E_OK)
    return res;
#endif

  /* Enable timer */
  reg->TCR = TCR_CEN;

  return E_OK;
}
/*----------------------------------------------------------------------------*/
#ifndef CONFIG_PLATFORM_LPC_GPTIMER_NO_DEINIT
static void unitDeinit(void *object)
{
  struct GpTimerPwmUnit * const unit = object;
  LPC_TIMER_Type * const reg = unit->base.reg;

  reg->TCR = 0;

#ifdef CONFIG_PLATFORM_LPC_GPTIMER_PM
  pmUnregister(unit);
#endif

  GpTimerBase->deinit(unit);
}
#endif
/*----------------------------------------------------------------------------*/
static enum Result channelInit(void *object, const void *configBase)
{
  const struct GpTimerPwmConfig * const config = configBase;
  assert(config);

  struct GpTimerPwm * const pwm = object;
  struct GpTimerPwmUnit * const unit = config->parent;

  /* Initialize output pin */
  pwm->channel = gpTimerConfigMatchPin(unit->base.channel, config->pin);

  /* Allocate channel */
  if (unitAllocateChannel(unit, pwm->channel))
  {
    LPC_TIMER_Type * const reg = unit->base.reg;

    pwm->unit = unit;
    pwm->value = reg->MR + pwm->channel;

    return E_OK;
  }
  else
    return E_BUSY;
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

  reg->PWMC &= ~PWMC_ENABLE(pwm->channel);
  /* Clear match value to avoid undefined output level */
  reg->EMR &= ~EMR_EXTERNAL_MATCH(pwm->channel);
}
/*----------------------------------------------------------------------------*/
static uint32_t channelGetResolution(const void *object)
{
  return ((const struct GpTimerPwm *)object)->unit->resolution;
}
/*----------------------------------------------------------------------------*/
static void channelSetDuration(void *object, uint32_t duration)
{
  struct GpTimerPwm * const pwm = object;
  const uint32_t resolution = pwm->unit->resolution;

  if (duration)
  {
    /* The output is inverted */
    duration = duration <= resolution ? resolution - duration : 0;
  }
  else
  {
    /*
     * If match register is set to a value greater or equal to resolution,
     * then the output stays low during all cycle.
     */
    duration = resolution + 1;
  }

  /*
   * If a match register is set to zero, than output pin goes high
   * and will stay in this state continuously.
   */
  *pwm->value = duration;
}
/*----------------------------------------------------------------------------*/
static void channelSetEdges(void *object,
    uint32_t leading __attribute__((unused)), uint32_t trailing)
{
  assert(leading == 0); /* Leading edge time must be zero */
  channelSetDuration(object, trailing);
}
/*----------------------------------------------------------------------------*/
static void channelSetFrequency(void *object, uint32_t frequency)
{
  struct GpTimerPwm * const pwm = object;
  struct GpTimerPwmUnit * const unit = pwm->unit;

  unit->frequency = frequency;
  gpTimerSetFrequency(&unit->base, unit->frequency * unit->resolution);
}
/*----------------------------------------------------------------------------*/
/**
 * Create single edge PWM channel.
 * @param unit Pointer to a GpTimerPwmUnit object.
 * @param pin Pin used as a signal output.
 * @return Pointer to a new Pwm object on success or zero on error.
 */
void *gpTimerPwmCreate(void *unit, PinNumber pin)
{
  const struct GpTimerPwmConfig channelConfig = {
      .parent = unit,
      .pin = pin
  };

  return init(GpTimerPwm, &channelConfig);
}
