/*
 * gptimer_pwm.c
 * Copyright (C) 2013 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <assert.h>
#include <platform/nxp/gptimer_pwm.h>
#include <platform/nxp/gptimer_pwm_defs.h>
/*----------------------------------------------------------------------------*/
#define UNPACK_FUNCTION(value)  ((value) & 0x0F)
/* Unpack timer match channel */
#define UNPACK_CHANNEL(value)   (((value) >> 4) & 0x0F)
/*----------------------------------------------------------------------------*/
static int8_t findEmptyChannel(uint8_t);
static void updateResolution(struct GpTimerPwmUnit *, uint8_t);
/*----------------------------------------------------------------------------*/
static enum result unitInit(void *, const void *);
static void unitDeinit(void *);
/*----------------------------------------------------------------------------*/
static enum result channelInit(void *, const void *);
static void channelDeinit(void *);
static uint32_t channelGetResolution(void *);
static void channelSetDuration(void *, uint32_t);
static void channelSetEdges(void *, uint32_t, uint32_t);
static void channelSetEnabled(void *, bool);
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
    .setEnabled = channelSetEnabled
};
/*----------------------------------------------------------------------------*/
extern const struct GpioDescriptor gpTimerPwmPins[];
const struct EntityClass *GpTimerPwmUnit = &unitTable;
const struct PwmClass *GpTimerPwm = &channelTable;
/*----------------------------------------------------------------------------*/
static int8_t findEmptyChannel(uint8_t channels)
{
  int8_t pos = 4; /* Each timer has 4 match blocks */

  while (--pos >= 0 && channels & (1 << pos));
  return pos;
}
/*----------------------------------------------------------------------------*/
static void updateResolution(struct GpTimerPwmUnit *unit, uint8_t channel)
{
  LPC_TIMER_Type *reg = unit->parent.reg;

  /* Put the timer into a reset state to clear prescaler and current value */
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
static enum result unitInit(void *object, const void *configPtr)
{
  const struct GpTimerPwmUnitConfig * const config = configPtr;
  const struct GpTimerBaseConfig parentConfig = {
      .channel = config->channel,
      .input = 0
  };
  struct GpTimerPwmUnit *unit = object;
  enum result res;

  const uint32_t clockFrequency = gpTimerGetClock(object);
  const uint32_t timerFrequency = config->frequency * config->resolution;
  assert(timerFrequency && timerFrequency < clockFrequency);

  /* Call base class constructor */
  if ((res = GpTimerBase->init(object, &parentConfig)) != E_OK)
    return res;

  unit->resolution = config->resolution;
  unit->matches = 0;
  unit->current = findEmptyChannel(unit->matches);

  LPC_TIMER_Type *reg = unit->parent.reg;

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
  struct GpTimerPwmUnit *unit = object;

  ((LPC_TIMER_Type *)unit->parent.reg)->TCR &= ~TCR_CEN;
  GpTimerBase->deinit(object);
}
/*----------------------------------------------------------------------------*/
static enum result channelInit(void *object, const void *configPtr)
{
  const struct GpTimerPwmConfig * const config = configPtr;
  const struct GpioDescriptor *pinDescriptor;
  struct GpTimerPwm *pwm = object;

  pinDescriptor = gpioFind(gpTimerPwmPins, config->pin,
      config->parent->parent.channel);
  if (!pinDescriptor)
    return E_VALUE;

  /* Check if there is a free match channel */
  int8_t freeChannel = findEmptyChannel(config->parent->matches
      | (1 << UNPACK_CHANNEL(pinDescriptor->value)));
  if (freeChannel == -1)
    return E_BUSY;

  pwm->channel = UNPACK_CHANNEL(pinDescriptor->value);
  pwm->unit = config->parent;
  pwm->unit->matches |= 1 << pwm->channel;

  /* Update match channel used for timer reset */
  updateResolution(pwm->unit, (uint8_t)freeChannel);

  /* Initialize match output pin */
  struct Gpio pin = gpioInit(config->pin);
  gpioOutput(pin, 0);
  gpioSetFunction(pin, UNPACK_FUNCTION(pinDescriptor->value));

  LPC_TIMER_Type *reg = pwm->unit->parent.reg;

  /* Calculate pointer to match register for fast access */
  pwm->value = (uint32_t *)(reg->MR + pwm->channel);
  /* Call function directly because of unfinished object construction */
  channelSetDuration(pwm, config->duration);
  /* Enable PWM channel */
  reg->PWMC |= PWMC_ENABLE(pwm->channel);

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static void channelDeinit(void *object)
{
  struct GpTimerPwm *pwm = object;
  LPC_TIMER_Type *reg = pwm->unit->parent.reg;

  reg->PWMC &= ~PWMC_ENABLE(pwm->channel);
}
/*----------------------------------------------------------------------------*/
static uint32_t channelGetResolution(void *object)
{
  return ((struct GpTimerPwm *)object)->unit->resolution;
}
/*----------------------------------------------------------------------------*/
static void channelSetDuration(void *object, uint32_t duration)
{
  struct GpTimerPwm *pwm = object;
  uint32_t value;

  /* Polarity is inverse */
  if (duration >= pwm->unit->resolution)
  {
    /*
     * If match register is set to a value greater or equal to resolution,
     * then output stays low during all cycle.
     */
    value = pwm->unit->resolution;
  }
  else
  {
    /*
     * If a match register is set to zero, than output pin goes high
     * and will stay in this state continuously.
     */
    value = !duration ? 0 : duration;
  }
  *pwm->value = value;
}
/*----------------------------------------------------------------------------*/
static void channelSetEdges(void *object,
    uint32_t leading __attribute__((unused)), uint32_t trailing)
{
  struct GpTimerPwm *pwm = object;
  uint32_t value;

  assert(!leading); /* Leading edge time is constant in single edge mode */

  if (trailing >= pwm->unit->resolution)
    value = pwm->unit->resolution;
  else
    value = !trailing ? 0 : trailing;
  *pwm->value = value;
}
/*----------------------------------------------------------------------------*/
static void channelSetEnabled(void *object, bool state)
{
  struct GpTimerPwm *pwm = object;
  LPC_TIMER_Type *reg = pwm->unit->parent.reg;

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
/**
 * Create single edge PWM channel with inverse polarity.
 * @param unit Pointer to GpTimerPwmUnit object.
 * @param pin Pin used as output for pulse width modulated signal.
 * @param duration Initial duration in timer ticks.
 * @return Pointer to new Pwm object on success or zero on error.
 */
void *gpTimerPwmCreate(void *unit, gpio_t pin, uint32_t duration)
{
  const struct GpTimerPwmConfig channelConfig = {
      .parent = unit,
      .duration = duration,
      .pin = pin
  };

  return init(GpTimerPwm, &channelConfig);
}
