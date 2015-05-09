/*
 * gptimer_pwm.c
 * Copyright (C) 2013 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <assert.h>
#include <platform/nxp/gptimer_pwm.h>
#include <platform/nxp/gptimer_pwm_defs.h>
/*----------------------------------------------------------------------------*/
static enum result unitAllocateChannel(struct GpTimerPwmUnit *, uint8_t);
static void unitReleaseChannel(struct GpTimerPwmUnit *, uint8_t);
static void unitUpdateResolution(struct GpTimerPwmUnit *, uint8_t);
static enum result unitInit(void *, const void *);
static void unitDeinit(void *);
/*----------------------------------------------------------------------------*/
static enum result channelInit(void *, const void *);
static void channelDeinit(void *);
static uint32_t channelGetResolution(const void *);
static void channelSetDuration(void *, uint32_t);
static void channelSetEdges(void *, uint32_t, uint32_t);
static void channelSetEnabled(void *, bool);
static enum result channelSetFrequency(void *, uint32_t);
/*----------------------------------------------------------------------------*/
static const struct EntityClass unitTable = {
    .size = sizeof(struct GpTimerPwmUnit),
    .init = unitInit,
    .deinit = unitDeinit
};
/*----------------------------------------------------------------------------*/
static const struct PwmClass channelTable = {
    .size = sizeof(struct GpTimerPwm),
    .init = channelInit,
    .deinit = channelDeinit,

    .getResolution = channelGetResolution,
    .setDuration = channelSetDuration,
    .setEdges = channelSetEdges,
    .setEnabled = channelSetEnabled,
    .setFrequency = channelSetFrequency
};
/*----------------------------------------------------------------------------*/
const struct EntityClass * const GpTimerPwmUnit = &unitTable;
const struct PwmClass * const GpTimerPwm = &channelTable;
/*----------------------------------------------------------------------------*/
static enum result unitAllocateChannel(struct GpTimerPwmUnit *unit,
    uint8_t channel)
{
  const uint8_t mask = 1 << channel;
  enum result res = E_BUSY;

  spinLock(&unit->spinlock);

  const int8_t freeChannel = gpTimerAllocateChannel(unit->matches | mask);

  if (freeChannel != -1 && !(unit->matches & mask))
  {
    unit->matches |= mask;
    unitUpdateResolution(unit, (uint8_t)freeChannel);
    res = E_OK;
  }

  spinUnlock(&unit->spinlock);
  return res;
}
/*----------------------------------------------------------------------------*/
static void unitReleaseChannel(struct GpTimerPwmUnit *unit, uint8_t channel)
{
  spinLock(&unit->spinlock);
  unit->matches &= ~(1 << channel);
  spinUnlock(&unit->spinlock);
}
/*----------------------------------------------------------------------------*/
static void unitUpdateResolution(struct GpTimerPwmUnit *unit, uint8_t channel)
{
  LPC_TIMER_Type * const reg = unit->parent.reg;

  /* Put the timer into a reset state to clear prescaler and counter */
  reg->TCR |= TCR_CRES;

  /* Disable previous match channel */
  reg->MCR &= ~MCR_RESET(unit->current);

  /* Initialize new match channel and enable it */
  unit->current = channel;
  reg->MR[unit->current] = unit->resolution - 1;
  reg->MCR |= MCR_RESET(unit->current);

  /* Clear reset bit and enable counting */
  reg->TCR &= ~TCR_CRES;
}
/*----------------------------------------------------------------------------*/
static enum result unitInit(void *object, const void *configBase)
{
  const struct GpTimerPwmUnitConfig * const config = configBase;
  const struct GpTimerBaseConfig parentConfig = {
      .channel = config->channel
  };
  struct GpTimerPwmUnit * const unit = object;
  enum result res;

  const uint32_t clockFrequency = gpTimerGetClock(object);
  const uint32_t timerFrequency = config->frequency * config->resolution;

  if (!timerFrequency || timerFrequency > clockFrequency)
    return E_VALUE;

  /* Call base class constructor */
  if ((res = GpTimerBase->init(object, &parentConfig)) != E_OK)
    return res;

  unit->matches = 0;
  unit->spinlock = SPIN_UNLOCKED;
  unit->resolution = config->resolution;

  /* Should be invoked after object initialization completion */
  unit->current = gpTimerAllocateChannel(unit->matches);

  LPC_TIMER_Type * const reg = unit->parent.reg;

  reg->TCR = 0;

  reg->IR = reg->IR; /* Clear pending interrupts */
  reg->PC = reg->TC = 0;
  reg->CTCR = 0;
  reg->CCR = 0;
  reg->EMR = 0;
  reg->PWMC = 0; /* Register is available only on specific parts */

  /* Configure timings */
  reg->PR = clockFrequency / timerFrequency - 1;
  reg->MR[unit->current] = unit->resolution - 1;
  reg->MCR = MCR_RESET(unit->current);

  /* Enable timer */
  reg->TCR = TCR_CEN;

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static void unitDeinit(void *object)
{
  struct GpTimerPwmUnit * const unit = object;
  LPC_TIMER_Type * const reg = unit->parent.reg;

  reg->TCR &= ~TCR_CEN;
  GpTimerBase->deinit(unit);
}
/*----------------------------------------------------------------------------*/
static enum result channelInit(void *object, const void *configBase)
{
  const struct GpTimerPwmConfig * const config = configBase;
  struct GpTimerPwm * const pwm = object;
  struct GpTimerPwmUnit * const unit = config->parent;
  enum result res;

  /* Initialize output pin */
  const int8_t channel = gpTimerConfigMatchPin(unit->parent.channel,
      config->pin);

  if (channel == -1)
    return E_VALUE;

  /* Allocate channel */
  if ((res = unitAllocateChannel(unit, (uint8_t)channel)) != E_OK)
    return res;

  pwm->channel = (uint8_t)channel;
  pwm->unit = unit;

  LPC_TIMER_Type * const reg = pwm->unit->parent.reg;

  /* Calculate pointer to match register for fast access */
  pwm->value = reg->MR + pwm->channel;
  /* Call function directly because of unfinished object construction */
  channelSetDuration(pwm, config->duration);
  /* Enable PWM channel */
  reg->PWMC |= PWMC_ENABLE(pwm->channel);

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static void channelDeinit(void *object)
{
  struct GpTimerPwm * const pwm = object;
  LPC_TIMER_Type * const reg = pwm->unit->parent.reg;

  reg->PWMC &= ~PWMC_ENABLE(pwm->channel);
  unitReleaseChannel(pwm->unit, pwm->channel);
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

  /* Polarity is inverted */
  if (duration >= pwm->unit->resolution)
  {
    /*
     * If match register is set to a value greater or equal to resolution,
     * then output stays low during all cycle.
     */
    duration = pwm->unit->resolution;
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
  struct GpTimerPwm * const pwm = object;

  assert(!leading); /* Leading edge time is constant in single edge mode */

  if (trailing >= pwm->unit->resolution)
    trailing = pwm->unit->resolution;

  *pwm->value = trailing;
}
/*----------------------------------------------------------------------------*/
static void channelSetEnabled(void *object, bool state)
{
  struct GpTimerPwm * const pwm = object;
  LPC_TIMER_Type * const reg = pwm->unit->parent.reg;

  if (!state)
  {
    reg->PWMC &= ~PWMC_ENABLE(pwm->channel);
    /* Clear match value to avoid undefined output level */
    reg->EMR &= ~EMR_EXTERNAL_MATCH(pwm->channel);
  }
  else
    reg->PWMC |= PWMC_ENABLE(pwm->channel);
}
/*----------------------------------------------------------------------------*/
static enum result channelSetFrequency(void *object, uint32_t frequency)
{
  struct GpTimerPwm * const pwm = object;
  struct GpTimerPwmUnit * const unit = pwm->unit;
  LPC_TIMER_Type * const reg = unit->parent.reg;

  const uint32_t clockFrequency = gpTimerGetClock((struct GpTimerBase *)unit);
  const uint32_t timerFrequency = frequency * unit->resolution;

  if (!timerFrequency || timerFrequency > clockFrequency)
    return E_VALUE;

  /* TODO Add scaling of timer match values */
  reg->PR = clockFrequency / timerFrequency - 1;
  return E_OK;
}
/*----------------------------------------------------------------------------*/
/**
 * Create single edge PWM channel with inverse polarity.
 * @param unit Pointer to a GpTimerPwmUnit object.
 * @param pin Pin used as output for pulse width modulated signal.
 * @param duration Initial duration in timer ticks.
 * @return Pointer to new Pwm object on success or zero on error.
 */
void *gpTimerPwmCreate(void *unit, pin_t pin, uint32_t duration)
{
  const struct GpTimerPwmConfig channelConfig = {
      .parent = unit,
      .duration = duration,
      .pin = pin
  };

  return init(GpTimerPwm, &channelConfig);
}
