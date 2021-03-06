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
static bool unitAllocateChannel(struct GpPwmUnit *, uint8_t);
static void unitSetFrequency(struct GpPwmUnit *, uint32_t);

#ifdef CONFIG_PLATFORM_LPC_GPTIMER_PM
static void powerStateHandler(void *, enum PmState);
#endif
#ifndef CONFIG_PLATFORM_LPC_GPPWM_NO_DEINIT
static void unitReleaseChannel(struct GpPwmUnit *, uint8_t);
#endif
/*----------------------------------------------------------------------------*/
static enum Result unitInit(void *, const void *);

#ifndef CONFIG_PLATFORM_LPC_GPPWM_NO_DEINIT
static void unitDeinit(void *);
#else
#define unitDeinit deletedDestructorTrap
#endif
/*----------------------------------------------------------------------------*/
static enum Result singleEdgeInit(void *, const void *);
static void singleEdgeEnable(void *);
static void singleEdgeDisable(void *);
static uint32_t singleEdgeGetResolution(const void *);
static void singleEdgeSetDuration(void *, uint32_t);
static void singleEdgeSetEdges(void *, uint32_t, uint32_t);
static void singleEdgeSetFrequency(void *, uint32_t);

#ifndef CONFIG_PLATFORM_LPC_GPPWM_NO_DEINIT
static void singleEdgeDeinit(void *);
#else
#define singleEdgeDeinit deletedDestructorTrap
#endif
/*----------------------------------------------------------------------------*/
static enum Result doubleEdgeInit(void *, const void *);
static void doubleEdgeEnable(void *);
static void doubleEdgeDisable(void *);
static uint32_t doubleEdgeGetResolution(const void *);
static void doubleEdgeSetDuration(void *, uint32_t);
static void doubleEdgeSetEdges(void *, uint32_t, uint32_t);
static void doubleEdgeSetFrequency(void *, uint32_t);

#ifndef CONFIG_PLATFORM_LPC_GPPWM_NO_DEINIT
static void doubleEdgeDeinit(void *);
#else
#define doubleEdgeDeinit deletedDestructorTrap
#endif
/*----------------------------------------------------------------------------*/
const struct EntityClass * const GpPwmUnit = &(const struct EntityClass){
    .size = sizeof(struct GpPwmUnit),
    .init = unitInit,
    .deinit = unitDeinit
};

const struct PwmClass * const GpPwm = &(const struct PwmClass){
    .size = sizeof(struct GpPwm),
    .init = singleEdgeInit,
    .deinit = singleEdgeDeinit,

    .enable = singleEdgeEnable,
    .disable = singleEdgeDisable,
    .getResolution = singleEdgeGetResolution,
    .setDuration = singleEdgeSetDuration,
    .setEdges = singleEdgeSetEdges,
    .setFrequency = singleEdgeSetFrequency
};

const struct PwmClass * const GpPwmDoubleEdge = &(const struct PwmClass){
    .size = sizeof(struct GpPwmDoubleEdge),
    .init = doubleEdgeInit,
    .deinit = doubleEdgeDeinit,

    .enable = doubleEdgeEnable,
    .disable = doubleEdgeDisable,
    .getResolution = doubleEdgeGetResolution,
    .setDuration = doubleEdgeSetDuration,
    .setEdges = doubleEdgeSetEdges,
    .setFrequency = doubleEdgeSetFrequency
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
  assert(pinEntry);

  const struct Pin pin = pinInit(key);

  pinOutput(pin, false);
  pinSetFunction(pin, UNPACK_FUNCTION(pinEntry->value));

  return UNPACK_CHANNEL(pinEntry->value);
}
/*----------------------------------------------------------------------------*/
#ifdef CONFIG_PLATFORM_LPC_GPTIMER_PM
static void powerStateHandler(void *object, enum PmState state)
{
  if (state == PM_ACTIVE)
  {
    struct GpPwmUnit * const unit = object;
    unitSetFrequency(unit, unit->frequency * unit->resolution);
  }
}
#endif
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
static void unitSetFrequency(struct GpPwmUnit *unit, uint32_t frequency)
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
static enum Result unitInit(void *object, const void *configBase)
{
  const struct GpPwmUnitConfig * const config = configBase;
  assert(config);
  assert(config->resolution >= 2);

  const struct GpPwmUnitBaseConfig baseConfig = {
      .channel = config->channel
  };
  struct GpPwmUnit * const unit = object;
  enum Result res;

  /* Call base class constructor */
  if ((res = GpPwmUnitBase->init(unit, &baseConfig)) != E_OK)
    return res;

  LPC_PWM_Type * const reg = unit->base.reg;

  reg->TCR = TCR_CRES;

  reg->IR = reg->IR; /* Clear pending interrupts */
  reg->CTCR = 0;
  reg->CCR = 0;
  reg->PCR = 0;

  /* Configure timings */
  unit->frequency = config->frequency;
  unit->resolution = config->resolution;
  unitSetFrequency(unit, unit->frequency * unit->resolution);

  unit->matches = 0;
  reg->MR0 = unit->resolution;
  reg->MCR = MCR_RESET(0);

#ifdef CONFIG_PLATFORM_LPC_GPTIMER_PM
  if ((res = pmRegister(powerStateHandler, unit)) != E_OK)
    return res;
#endif

  /* Switch to the PWM mode and enable the timer */
  reg->TCR = TCR_CEN | TCR_PWM_ENABLE;

  return E_OK;
}
/*----------------------------------------------------------------------------*/
#ifndef CONFIG_PLATFORM_LPC_GPPWM_NO_DEINIT
static void unitDeinit(void *object)
{
  struct GpPwmUnit * const unit = object;
  LPC_PWM_Type * const reg = unit->base.reg;

  reg->TCR = 0;

#ifdef CONFIG_PLATFORM_LPC_GPTIMER_PM
  pmUnregister(unit);
#endif

  GpPwmUnitBase->deinit(unit);
}
#endif
/*----------------------------------------------------------------------------*/
static enum Result singleEdgeInit(void *object, const void *configBase)
{
  const struct GpPwmConfig * const config = configBase;
  assert(config);

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
static uint32_t singleEdgeGetResolution(const void *object)
{
  return ((const struct GpPwm *)object)->unit->resolution;
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
static void singleEdgeSetFrequency(void *object, uint32_t frequency)
{
  struct GpPwm * const pwm = object;
  struct GpPwmUnit * const unit = pwm->unit;

  unit->frequency = frequency;
  unitSetFrequency(unit, unit->frequency * unit->resolution);
}
/*----------------------------------------------------------------------------*/
static enum Result doubleEdgeInit(void *object, const void *configBase)
{
  const struct GpPwmDoubleEdgeConfig * const config = configBase;
  assert(config);

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
static uint32_t doubleEdgeGetResolution(const void *object)
{
  return ((const struct GpPwmDoubleEdge *)object)->unit->resolution;
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
static void doubleEdgeSetFrequency(void *object, uint32_t frequency)
{
  struct GpPwmDoubleEdge * const pwm = object;
  struct GpPwmUnit * const unit = pwm->unit;

  unit->frequency = frequency;
  unitSetFrequency(unit, unit->frequency * unit->resolution);
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
