/*
 * gppwm.c
 * Copyright (C) 2013 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/platform/lpc/gppwm.h>
#include <halm/platform/lpc/gppwm_defs.h>
#include <halm/pm.h>
#include <assert.h>
/*----------------------------------------------------------------------------*/
#define UNPACK_CHANNEL(value)   (((value) >> 4) & 0x0F)
#define UNPACK_FUNCTION(value)  ((value) & 0x0F)
/*----------------------------------------------------------------------------*/
static inline volatile uint32_t *calcMatchChannel(LPC_PWM_Type *, uint8_t);
static uint8_t configMatchPin(uint8_t channel, PinNumber key);
static void interruptHandler(void *);
static void setTimerFrequency(struct GpPwmUnit *, uint32_t);
static bool unitAllocateChannel(struct GpPwmUnit *, uint8_t);

#ifdef CONFIG_PLATFORM_LPC_GPPWM_PM
static void powerStateHandler(void *, enum PmState);
#endif
#ifndef CONFIG_PLATFORM_LPC_GPPWM_NO_DEINIT
static void unitReleaseChannel(struct GpPwmUnit *, uint8_t);
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

#ifndef CONFIG_PLATFORM_LPC_GPPWM_NO_DEINIT
static void unitDeinit(void *);
#else
#define unitDeinit deletedDestructorTrap
#endif
/*----------------------------------------------------------------------------*/
static enum Result singleEdgeInit(void *, const void *);
static void singleEdgeEnable(void *);
static void singleEdgeDisable(void *);
static void singleEdgeSetDuration(void *, uint32_t);
static void singleEdgeSetEdges(void *, uint32_t, uint32_t);

#ifndef CONFIG_PLATFORM_LPC_GPPWM_NO_DEINIT
static void singleEdgeDeinit(void *);
#else
#define singleEdgeDeinit deletedDestructorTrap
#endif
/*----------------------------------------------------------------------------*/
static enum Result doubleEdgeInit(void *, const void *);
static void doubleEdgeEnable(void *);
static void doubleEdgeDisable(void *);
static void doubleEdgeSetDuration(void *, uint32_t);
static void doubleEdgeSetEdges(void *, uint32_t, uint32_t);

#ifndef CONFIG_PLATFORM_LPC_GPPWM_NO_DEINIT
static void doubleEdgeDeinit(void *);
#else
#define doubleEdgeDeinit deletedDestructorTrap
#endif
/*----------------------------------------------------------------------------*/
const struct TimerClass * const GpPwmUnit = &(const struct TimerClass){
    .size = sizeof(struct GpPwmUnit),
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

const struct PwmClass * const GpPwm = &(const struct PwmClass){
    .size = sizeof(struct GpPwm),
    .init = singleEdgeInit,
    .deinit = singleEdgeDeinit,

    .enable = singleEdgeEnable,
    .disable = singleEdgeDisable,
    .setDuration = singleEdgeSetDuration,
    .setEdges = singleEdgeSetEdges
};

const struct PwmClass * const GpPwmDoubleEdge = &(const struct PwmClass){
    .size = sizeof(struct GpPwmDoubleEdge),
    .init = doubleEdgeInit,
    .deinit = doubleEdgeDeinit,

    .enable = doubleEdgeEnable,
    .disable = doubleEdgeDisable,
    .setDuration = doubleEdgeSetDuration,
    .setEdges = doubleEdgeSetEdges
};
/*----------------------------------------------------------------------------*/
extern const struct PinEntry gpPwmPins[];
/*----------------------------------------------------------------------------*/
static inline volatile uint32_t *calcMatchChannel(LPC_PWM_Type *device,
    uint8_t channel)
{
  assert(channel && channel <= 6);

  if (channel <= 3)
    return &device->MR1 + (channel - 1);
  else
    return &device->MR4 + (channel - 4);
}
/*----------------------------------------------------------------------------*/
static uint8_t configMatchPin(uint8_t channel, PinNumber key)
{
  const struct PinEntry * const pinEntry = pinFind(gpPwmPins, key, channel);
  assert(pinEntry != NULL);

  const struct Pin pin = pinInit(key);

  pinOutput(pin, false);
  pinSetFunction(pin, UNPACK_FUNCTION(pinEntry->value));

  return UNPACK_CHANNEL(pinEntry->value);
}
/*----------------------------------------------------------------------------*/
static void interruptHandler(void *object)
{
  struct GpPwmUnit * const unit = object;
  LPC_PWM_Type * const reg = unit->base.reg;

  /* Clear pending Match 0 interrupt */
  reg->IR = PWM_IR_MATCH_INTERRUPT(0);

  unit->callback(unit->callbackArgument);
}
/*----------------------------------------------------------------------------*/
#ifdef CONFIG_PLATFORM_LPC_GPPWM_PM
static void powerStateHandler(void *object, enum PmState state)
{
  if (state == PM_ACTIVE)
  {
    struct GpPwmUnit * const unit = object;
    setTimerFrequency(unit, unit->frequency);
  }
}
#endif
/*----------------------------------------------------------------------------*/
static void setTimerFrequency(struct GpPwmUnit *unit, uint32_t frequency)
{
  LPC_PWM_Type * const reg = unit->base.reg;

  if (frequency)
  {
    const uint32_t apbClock = gpPwmGetClock(&unit->base);
    const uint32_t divisor = apbClock / frequency - 1;

    assert(frequency <= apbClock);

    reg->PR = divisor;
  }
  else
    reg->PR = 0;
}
/*----------------------------------------------------------------------------*/
static bool unitAllocateChannel(struct GpPwmUnit *unit, uint8_t channel)
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
#ifndef CONFIG_PLATFORM_LPC_GPPWM_NO_DEINIT
static void unitReleaseChannel(struct GpPwmUnit *unit, uint8_t channel)
{
  unit->matches &= ~(1 << channel);
}
#endif
/*----------------------------------------------------------------------------*/
static enum Result unitInit(void *object, const void *configBase)
{
  const struct GpPwmUnitConfig * const config = configBase;
  assert(config != NULL);
  assert(config->resolution >= 2);

  const struct GpPwmUnitBaseConfig baseConfig = {
      .channel = config->channel
  };
  struct GpPwmUnit * const unit = object;
  enum Result res;

  /* Call base class constructor */
  if ((res = GpPwmUnitBase->init(unit, &baseConfig)) != E_OK)
    return res;

  unit->base.handler = interruptHandler;
  unit->callback = NULL;
  unit->frequency = config->frequency;
  unit->resolution = config->resolution;

  LPC_PWM_Type * const reg = unit->base.reg;

  reg->TCR = TCR_CRES;

  /* Clear pending interrupts */
  reg->IR = reg->IR;

  reg->CTCR = 0;
  reg->CCR = 0;
  reg->PCR = 0;

  /* Configure timings */
  setTimerFrequency(unit, unit->frequency);

  unit->matches = 0;
  reg->MR0 = unit->resolution;
  reg->MCR = MCR_RESET(0);

#ifdef CONFIG_PLATFORM_LPC_GPPWM_PM
  if ((res = pmRegister(powerStateHandler, unit)) != E_OK)
    return res;
#endif

  irqSetPriority(unit->base.irq, config->priority);
  irqEnable(unit->base.irq);

  /* Timer is left in a disabled state */
  return E_OK;
}
/*----------------------------------------------------------------------------*/
#ifndef CONFIG_PLATFORM_LPC_GPPWM_NO_DEINIT
static void unitDeinit(void *object)
{
  struct GpPwmUnit * const unit = object;
  LPC_PWM_Type * const reg = unit->base.reg;

  irqDisable(unit->base.irq);
  reg->TCR = 0;

#ifdef CONFIG_PLATFORM_LPC_GPPWM_PM
  pmUnregister(unit);
#endif

  GpPwmUnitBase->deinit(unit);
}
#endif
/*----------------------------------------------------------------------------*/
static void unitSetCallback(void *object, void (*callback)(void *),
    void *argument)
{
  struct GpPwmUnit * const unit = object;
  LPC_PWM_Type * const reg = unit->base.reg;

  unit->callbackArgument = argument;
  unit->callback = callback;

  if (unit->callback != NULL)
  {
    reg->IR = PWM_IR_MATCH_INTERRUPT(0);
    reg->MCR |= MCR_INTERRUPT(0);
  }
  else
    reg->MCR &= ~MCR_INTERRUPT(0);
}
/*----------------------------------------------------------------------------*/
static void unitEnable(void *object)
{
  struct GpPwmUnit * const unit = object;
  LPC_PWM_Type * const reg = unit->base.reg;

  /* Clear pending interrupt flags */
  reg->IR = reg->IR;
  /* Switch to the PWM mode and enable the timer */
  reg->TCR = TCR_CEN | TCR_PWM_ENABLE;
}
/*----------------------------------------------------------------------------*/
static void unitDisable(void *object)
{
  struct GpPwmUnit * const unit = object;
  LPC_PWM_Type * const reg = unit->base.reg;

  /* Stop the timer */
  reg->TCR = 0;
}
/*----------------------------------------------------------------------------*/
static uint32_t unitGetFrequency(const void *object)
{
  const struct GpPwmUnit * const unit = object;
  return unit->frequency;
}
/*----------------------------------------------------------------------------*/
static void unitSetFrequency(void *object, uint32_t frequency)
{
  struct GpPwmUnit * const unit = object;

  unit->frequency = frequency;
  setTimerFrequency(unit, unit->frequency);
}
/*----------------------------------------------------------------------------*/
static uint32_t unitGetOverflow(const void *object)
{
  const struct GpPwmUnit * const unit = object;
  return unit->resolution;
}
/*----------------------------------------------------------------------------*/
static void unitSetOverflow(void *object, uint32_t overflow)
{
  struct GpPwmUnit * const unit = object;
  LPC_PWM_Type * const reg = unit->base.reg;

  unit->resolution = overflow;
  reg->MR0 = unit->resolution;
}
/*----------------------------------------------------------------------------*/
static enum Result singleEdgeInit(void *object, const void *configBase)
{
  const struct GpPwmConfig * const config = configBase;
  assert(config != NULL);

  struct GpPwm * const pwm = object;
  struct GpPwmUnit * const unit = config->parent;

  /* Initialize output pin */
  pwm->channel = configMatchPin(unit->base.channel, config->pin);

  /* Allocate channel */
  if (unitAllocateChannel(unit, pwm->channel))
  {
    LPC_PWM_Type * const reg = unit->base.reg;

    /* Select single edge mode */
    reg->PCR &= ~PCR_DOUBLE_EDGE(pwm->channel);

    pwm->unit = unit;
    pwm->latch = LER_ENABLE(pwm->channel);

    /* Calculate pointer to the match register */
    pwm->value = calcMatchChannel(reg, pwm->channel);

    return E_OK;
  }
  else
    return E_BUSY;
}
/*----------------------------------------------------------------------------*/
#ifndef CONFIG_PLATFORM_LPC_GPPWM_NO_DEINIT
static void singleEdgeDeinit(void *object)
{
  struct GpPwm * const pwm = object;
  LPC_PWM_Type * const reg = pwm->unit->base.reg;

  reg->PCR &= ~PCR_OUTPUT_ENABLED(pwm->channel);
  unitReleaseChannel(pwm->unit, pwm->channel);
}
#endif
/*----------------------------------------------------------------------------*/
static void singleEdgeEnable(void *object)
{
  struct GpPwm * const pwm = object;
  LPC_PWM_Type * const reg = pwm->unit->base.reg;

  reg->PCR |= PCR_OUTPUT_ENABLED(pwm->channel);
}
/*----------------------------------------------------------------------------*/
static void singleEdgeDisable(void *object)
{
  struct GpPwm * const pwm = object;
  LPC_PWM_Type * const reg = pwm->unit->base.reg;

  reg->PCR &= ~PCR_OUTPUT_ENABLED(pwm->channel);
}
/*----------------------------------------------------------------------------*/
static void singleEdgeSetDuration(void *object, uint32_t duration)
{
  struct GpPwm * const pwm = object;
  LPC_PWM_Type * const reg = pwm->unit->base.reg;

  /*
   * If match register is set to a value greater than resolution,
   * than output stays high during all cycle.
   */
  *pwm->value = duration;
  reg->LER |= pwm->latch;
}
/*----------------------------------------------------------------------------*/
static void singleEdgeSetEdges(void *object,
    uint32_t leading __attribute__((unused)), uint32_t trailing)
{
  assert(leading == 0); /* Leading edge time must be zero */
  singleEdgeSetDuration(object, trailing);
}
/*----------------------------------------------------------------------------*/
static enum Result doubleEdgeInit(void *object, const void *configBase)
{
  const struct GpPwmDoubleEdgeConfig * const config = configBase;
  assert(config != NULL);

  struct GpPwmDoubleEdge * const pwm = object;
  struct GpPwmUnit * const unit = config->parent;

  /* Initialize output pin */
  pwm->channel = configMatchPin(unit->base.channel, config->pin);
  /* First channel cannot be a double edged output */
  assert(pwm->channel > 1);

  /* Allocate channel */
  if (unitAllocateChannel(unit, pwm->channel - 1)
      && unitAllocateChannel(unit, pwm->channel))
  {
    LPC_PWM_Type * const reg = unit->base.reg;

    /* Select double edge mode */
    reg->PCR |= PCR_DOUBLE_EDGE(pwm->channel);

    pwm->unit = unit;
    pwm->latch = LER_ENABLE(pwm->channel) | LER_ENABLE(pwm->channel - 1);

    /* Calculate pointers to the match registers */
    pwm->leading = calcMatchChannel(reg, pwm->channel - 1);
    pwm->trailing = calcMatchChannel(reg, pwm->channel);

    return E_OK;
  }
  else
    return E_BUSY;
}
/*----------------------------------------------------------------------------*/
#ifndef CONFIG_PLATFORM_LPC_GPPWM_NO_DEINIT
static void doubleEdgeDeinit(void *object)
{
  struct GpPwmDoubleEdge * const pwm = object;
  LPC_PWM_Type * const reg = pwm->unit->base.reg;

  reg->PCR &= ~PCR_OUTPUT_ENABLED(pwm->channel);
  unitReleaseChannel(pwm->unit, pwm->channel);
  unitReleaseChannel(pwm->unit, pwm->channel - 1);
}
#endif
/*----------------------------------------------------------------------------*/
static void doubleEdgeEnable(void *object)
{
  struct GpPwmDoubleEdge * const pwm = object;
  LPC_PWM_Type * const reg = pwm->unit->base.reg;

  reg->PCR |= PCR_OUTPUT_ENABLED(pwm->channel);
}
/*----------------------------------------------------------------------------*/
static void doubleEdgeDisable(void *object)
{
  struct GpPwmDoubleEdge * const pwm = object;
  LPC_PWM_Type * const reg = pwm->unit->base.reg;

  reg->PCR &= ~PCR_OUTPUT_ENABLED(pwm->channel);
}
/*----------------------------------------------------------------------------*/
static void doubleEdgeSetDuration(void *object, uint32_t duration)
{
  struct GpPwmDoubleEdge * const pwm = object;
  const uint32_t resolution = pwm->unit->resolution;
  const uint32_t leading = *pwm->leading;
  uint32_t trailing;

  if (duration >= resolution)
  {
    trailing = resolution;
  }
  else
  {
    if (leading >= resolution - duration)
      trailing = leading - (resolution - duration);
    else
      trailing = leading + duration;
  }

  doubleEdgeSetEdges(pwm, leading, trailing);
}
/*----------------------------------------------------------------------------*/
static void doubleEdgeSetEdges(void *object, uint32_t leading,
    uint32_t trailing)
{
  struct GpPwmDoubleEdge * const pwm = object;
  LPC_PWM_Type * const reg = pwm->unit->base.reg;

  /*
   * If a match value is greater than the PWM rate value, no match event
   * occurs and the PWM output will remain always in one state.
   */
  *pwm->leading = leading;
  *pwm->trailing = trailing;
  reg->LER |= pwm->latch;
}
/*----------------------------------------------------------------------------*/
/**
 * Create single edge PWM channel.
 * @param unit Pointer to a GpPwmUnit object.
 * @param pin Pin used as a signal output.
 * @return Pointer to a new Pwm object on success or zero on error.
 */
void *gpPwmCreate(void *unit, PinNumber pin)
{
  const struct GpPwmConfig channelConfig = {
      .parent = unit,
      .pin = pin
  };

  return init(GpPwm, &channelConfig);
}
/*----------------------------------------------------------------------------*/
/**
 * Create double edge PWM channel.
 * @param unit Pointer to a GpPwmUnit object.
 * @param pin Pin used as a signal output.
 * @return Pointer to a new Pwm object on success or zero on error.
 */
void *gpPwmCreateDoubleEdge(void *unit, PinNumber pin)
{
  const struct GpPwmDoubleEdgeConfig channelConfig = {
      .parent = unit,
      .pin = pin
  };

  return init(GpPwmDoubleEdge, &channelConfig);
}
