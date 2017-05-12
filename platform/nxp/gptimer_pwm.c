/*
 * gptimer_pwm.c
 * Copyright (C) 2013 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <assert.h>
#include <halm/platform/nxp/gptimer_pwm.h>
#include <halm/platform/nxp/gptimer_pwm_defs.h>
/*----------------------------------------------------------------------------*/
static enum Result unitAllocateChannel(struct GpTimerPwmUnit *, uint8_t);
static void unitReleaseChannel(struct GpTimerPwmUnit *, uint8_t);
static void unitUpdateResolution(struct GpTimerPwmUnit *, uint8_t);
static enum Result unitInit(void *, const void *);
static void unitDeinit(void *);
/*----------------------------------------------------------------------------*/
static enum Result channelInit(void *, const void *);
static void channelDeinit(void *);
static void channelEnable(void *);
static void channelDisable(void *);
static uint32_t channelGetResolution(const void *);
static void channelSetDuration(void *, uint32_t);
static void channelSetEdges(void *, uint32_t, uint32_t);
static enum Result channelSetFrequency(void *, uint32_t);
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

    .enable = channelEnable,
    .disable = channelDisable,
    .getResolution = channelGetResolution,
    .setDuration = channelSetDuration,
    .setEdges = channelSetEdges,
    .setFrequency = channelSetFrequency
};
/*----------------------------------------------------------------------------*/
const struct EntityClass * const GpTimerPwmUnit = &unitTable;
const struct PwmClass * const GpTimerPwm = &channelTable;
/*----------------------------------------------------------------------------*/
static enum Result unitAllocateChannel(struct GpTimerPwmUnit *unit,
    uint8_t channel)
{
  enum Result res = E_BUSY;
  const uint8_t mask = 1 << channel;
  const IrqState state = irqSave();

  const int freeChannel = gpTimerAllocateChannel(unit->matches | mask);

  if (freeChannel != -1 && !(unit->matches & mask))
  {
    unit->matches |= mask;
    unitUpdateResolution(unit, freeChannel);
    res = E_OK;
  }

  irqRestore(state);
  return res;
}
/*----------------------------------------------------------------------------*/
static void unitReleaseChannel(struct GpTimerPwmUnit *unit, uint8_t channel)
{
  const IrqState state = irqSave();

  unit->matches &= ~(1 << channel);

  irqRestore(state);
}
/*----------------------------------------------------------------------------*/
static void unitUpdateResolution(struct GpTimerPwmUnit *unit, uint8_t channel)
{
  LPC_TIMER_Type * const reg = unit->base.reg;

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
static enum Result unitInit(void *object, const void *configBase)
{
  const struct GpTimerPwmUnitConfig * const config = configBase;
  assert(config);

  const struct GpTimerBaseConfig baseConfig = {
      .channel = config->channel
  };
  struct GpTimerPwmUnit * const unit = object;
  enum Result res;

  const uint32_t clockFrequency = gpTimerGetClock(object);
  const uint32_t timerFrequency = config->frequency * config->resolution;

  if (!timerFrequency || timerFrequency > clockFrequency)
    return E_VALUE;

  /* Call base class constructor */
  if ((res = GpTimerBase->init(object, &baseConfig)) != E_OK)
    return res;

  unit->matches = 0;
  unit->resolution = config->resolution;

  /* Should be invoked after object initialization completion */
  unit->current = gpTimerAllocateChannel(unit->matches);

  LPC_TIMER_Type * const reg = unit->base.reg;

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
  LPC_TIMER_Type * const reg = unit->base.reg;

  reg->TCR &= ~TCR_CEN;
  GpTimerBase->deinit(unit);
}
/*----------------------------------------------------------------------------*/
static enum Result channelInit(void *object, const void *configBase)
{
  const struct GpTimerPwmConfig * const config = configBase;
  assert(config);

  struct GpTimerPwm * const pwm = object;
  struct GpTimerPwmUnit * const unit = config->parent;
  enum Result res;

  /* Initialize output pin */
  pwm->channel = gpTimerConfigMatchPin(unit->base.channel, config->pin);

  /* Allocate channel */
  if ((res = unitAllocateChannel(unit, pwm->channel)) != E_OK)
    return res;

  pwm->unit = unit;

  LPC_TIMER_Type * const reg = pwm->unit->base.reg;

  /* Calculate pointer to match register for fast access */
  pwm->value = reg->MR + pwm->channel;

  /* Set initial duration */
  channelSetDuration(pwm, 0);

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static void channelDeinit(void *object)
{
  struct GpTimerPwm * const pwm = object;

  channelDisable(pwm);
  unitReleaseChannel(pwm->unit, pwm->channel);
}
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
    if (duration > resolution)
      duration = resolution;

    /* The output is inverted */
    duration = resolution - duration;
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
  /* Leading edge time is constant in the single edge mode */
  assert(leading == 0);

  channelSetDuration(object, trailing);
}
/*----------------------------------------------------------------------------*/
static enum Result channelSetFrequency(void *object, uint32_t frequency)
{
  struct GpTimerPwm * const pwm = object;
  struct GpTimerPwmUnit * const unit = pwm->unit;
  LPC_TIMER_Type * const reg = unit->base.reg;

  const uint32_t clockFrequency = gpTimerGetClock(&unit->base);
  const uint32_t timerFrequency = frequency * unit->resolution;

  if (!timerFrequency || timerFrequency > clockFrequency)
    return E_VALUE;

  /* TODO Add scaling of timer match values */
  reg->PR = clockFrequency / timerFrequency - 1;

  return E_OK;
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
