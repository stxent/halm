/*
 * sct_pwm.c
 * Copyright (C) 2015 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <assert.h>
#include <halm/platform/nxp/sct_defs.h>
#include <halm/platform/nxp/sct_pwm.h>
/*----------------------------------------------------------------------------*/
#define UNPACK_CHANNEL(value)   (((value) >> 4) & 0x0F)
#define UNPACK_FUNCTION(value)  ((value) & 0x0F)
/*----------------------------------------------------------------------------*/
static uint8_t configOutputPin(uint8_t, PinNumber);
static void setUnitResolution(struct SctPwmUnit *, uint8_t, uint32_t);
/*----------------------------------------------------------------------------*/
static enum Result unitInit(void *, const void *);

#ifndef CONFIG_PLATFORM_NXP_SCT_NO_DEINIT
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
static void singleEdgeSetDurationUnified(void *, uint32_t);
static void singleEdgeSetEdges(void *, uint32_t, uint32_t);
static void singleEdgeSetEdgesUnified(void *, uint32_t, uint32_t);
static void singleEdgeSetFrequency(void *, uint32_t);

#ifndef CONFIG_PLATFORM_NXP_SCT_NO_DEINIT
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
static void doubleEdgeSetDurationUnified(void *, uint32_t);
static void doubleEdgeSetEdges(void *, uint32_t, uint32_t);
static void doubleEdgeSetEdgesUnified(void *, uint32_t, uint32_t);
static void doubleEdgeSetFrequency(void *, uint32_t);

#ifndef CONFIG_PLATFORM_NXP_SCT_NO_DEINIT
static void doubleEdgeDeinit(void *);
#else
#define doubleEdgeDeinit deletedDestructorTrap
#endif
/*----------------------------------------------------------------------------*/
static const struct EntityClass unitTable = {
    .size = sizeof(struct SctPwmUnit),
    .init = unitInit,
    .deinit = unitDeinit
};

static const struct PwmClass singleEdgeTable = {
    .size = sizeof(struct SctPwm),
    .init = singleEdgeInit,
    .deinit = singleEdgeDeinit,

    .enable = singleEdgeEnable,
    .disable = singleEdgeDisable,
    .getResolution = singleEdgeGetResolution,
    .setDuration = singleEdgeSetDuration,
    .setEdges = singleEdgeSetEdges,
    .setFrequency = singleEdgeSetFrequency
};

static const struct PwmClass singleEdgeUnifiedTable = {
    .size = sizeof(struct SctPwm),
    .init = singleEdgeInit,
    .deinit = singleEdgeDeinit,

    .enable = singleEdgeEnable,
    .disable = singleEdgeDisable,
    .getResolution = singleEdgeGetResolution,
    .setDuration = singleEdgeSetDurationUnified,
    .setEdges = singleEdgeSetEdgesUnified,
    .setFrequency = singleEdgeSetFrequency
};

static const struct PwmClass doubleEdgeTable = {
    .size = sizeof(struct SctPwm),
    .init = doubleEdgeInit,
    .deinit = doubleEdgeDeinit,

    .enable = doubleEdgeEnable,
    .disable = doubleEdgeDisable,
    .getResolution = doubleEdgeGetResolution,
    .setDuration = doubleEdgeSetDuration,
    .setEdges = doubleEdgeSetEdges,
    .setFrequency = doubleEdgeSetFrequency
};

static const struct PwmClass doubleEdgeUnifiedTable = {
    .size = sizeof(struct SctPwm),
    .init = doubleEdgeInit,
    .deinit = doubleEdgeDeinit,

    .enable = doubleEdgeEnable,
    .disable = doubleEdgeDisable,
    .getResolution = doubleEdgeGetResolution,
    .setDuration = doubleEdgeSetDurationUnified,
    .setEdges = doubleEdgeSetEdgesUnified,
    .setFrequency = doubleEdgeSetFrequency
};
/*----------------------------------------------------------------------------*/
extern const struct PinEntry sctOutputPins[];
const struct EntityClass * const SctPwmUnit = &unitTable;
const struct PwmClass * const SctPwm = &singleEdgeTable;
const struct PwmClass * const SctUnifiedPwm = &singleEdgeUnifiedTable;
const struct PwmClass * const SctPwmDoubleEdge = &doubleEdgeTable;
const struct PwmClass * const SctUnifiedPwmDoubleEdge = &doubleEdgeUnifiedTable;
/*----------------------------------------------------------------------------*/
static uint8_t configOutputPin(uint8_t channel, PinNumber key)
{
  const struct PinEntry * const pinEntry = pinFind(sctOutputPins, key, channel);
  assert(pinEntry);

  const struct Pin pin = pinInit(key);

  pinOutput(pin, false);
  pinSetFunction(pin, UNPACK_FUNCTION(pinEntry->value));

  return UNPACK_CHANNEL(pinEntry->value);
}
/*----------------------------------------------------------------------------*/
static void setUnitResolution(struct SctPwmUnit *unit, uint8_t channel,
    uint32_t value)
{
  const unsigned int part = unit->base.part == SCT_HIGH;
  LPC_SCT_Type * const reg = unit->base.reg;

  reg->CONFIG |= CONFIG_NORELOAD(part);

  if (unit->base.part != SCT_UNIFIED)
  {
    assert(value <= 0xFFFF);

    reg->MATCH_PART[channel][part] = value;
    reg->MATCHREL_PART[channel][part] = value;
  }
  else
  {
    /*
     * Max 32-bit value is reserved for the case when a PWM output
     * should stay high during all cycle.
     */
    assert(value < 0xFFFFFFFFUL);

    reg->MATCH[channel] = value;
    reg->MATCHREL[channel] = value;
  }

  reg->CONFIG &= ~CONFIG_NORELOAD(part);
}
/*----------------------------------------------------------------------------*/
static enum Result unitInit(void *object, const void *configBase)
{
  const struct SctPwmUnitConfig * const config = configBase;
  assert(config);
  assert(config->resolution >= 2);

  const struct SctBaseConfig baseConfig = {
      .channel = config->channel,
      .edge = PIN_RISING,
      .input = SCT_INPUT_NONE,
      .part = config->part
  };
  struct SctPwmUnit * const unit = object;
  enum Result res;

  /* Call base class constructor */
  if ((res = SctBase->init(object, &baseConfig)) != E_OK)
    return res;

  if (!sctAllocateEvent(&unit->base, &unit->event))
    return E_BUSY;

  LPC_SCT_Type * const reg = unit->base.reg;
  const unsigned int part = unit->base.part == SCT_HIGH;
  const uint16_t eventMask = 1 << unit->event;

  unit->base.mask = eventMask;
  reg->CTRL_PART[part] = CTRL_HALT;

  /* Set desired unit frequency */
  unit->frequency = config->frequency;
  unit->resolution = config->resolution;
  sctSetFrequency(&unit->base, unit->frequency * unit->resolution);

  reg->CONFIG &= ~(CONFIG_AUTOLIMIT(part) | CONFIG_NORELOAD(part));

  /* Match value should be configured after event initialization */
  setUnitResolution(unit, unit->event, unit->resolution - 1);

  /* Configure event */
  reg->EV[unit->event].CTRL = EVCTRL_MATCHSEL(unit->event)
      | EVCTRL_COMBMODE(COMBMODE_MATCH) | EVCTRL_MATCHMEM
      | EVCTRL_DIRECTION(DIRECTION_INDEPENDENT);
  if (unit->base.part == SCT_HIGH)
    reg->EV[unit->event].CTRL |= EVCTRL_HEVENT;

  /* Reset current state and enable allocated event in state 0 */
  reg->STATE_PART[part] = 0;
  reg->EV[unit->event].STATE = 0x00000001;
  /* Enable timer clearing on allocated event */
  reg->LIMIT_PART[part] = eventMask;

  /* Enable counter */
  reg->CTRL_PART[part] &= ~CTRL_HALT;

  return E_OK;
}
/*----------------------------------------------------------------------------*/
#ifndef CONFIG_PLATFORM_NXP_SCT_NO_DEINIT
static void unitDeinit(void *object)
{
  struct SctPwmUnit * const unit = object;
  const unsigned int part = unit->base.part == SCT_HIGH;
  LPC_SCT_Type * const reg = unit->base.reg;

  /* Halt the timer */
  reg->CTRL_PART[part] = CTRL_HALT;
  reg->LIMIT_PART[part] = 0;

  /* Disable allocated event */
  reg->EV[unit->event].CTRL = 0;
  reg->EV[unit->event].STATE = 0;
  sctReleaseEvent(&unit->base, unit->event);

  SctBase->deinit(unit);
}
#endif
/*----------------------------------------------------------------------------*/
static enum Result singleEdgeInit(void *object, const void *configBase)
{
  const struct SctPwmConfig * const config = configBase;
  assert(config);

  struct SctPwm * const pwm = object;
  struct SctPwmUnit * const unit = config->parent;

  if (!sctAllocateEvent(&unit->base, &pwm->event))
    return E_BUSY;

  /* Initialize output pin */
  pwm->channel = configOutputPin(unit->base.channel, config->pin);
  pwm->unit = unit;

  LPC_SCT_Type * const reg = unit->base.reg;

  if (unit->base.part == SCT_UNIFIED)
    pwm->value = &reg->MATCHREL[pwm->event];
  else
    pwm->value = &reg->MATCHREL_PART[pwm->event][unit->base.part];

  /* Configure event */
  reg->EV[pwm->event].CTRL = EVCTRL_MATCHSEL(pwm->event)
      | EVCTRL_OUTSEL | EVCTRL_IOSEL(pwm->channel)
      | EVCTRL_COMBMODE(COMBMODE_MATCH)
      | EVCTRL_DIRECTION(DIRECTION_INDEPENDENT);

  if (unit->base.part == SCT_HIGH)
    reg->EV[pwm->event].CTRL |= EVCTRL_HEVENT;

  /* Clear the output when both events occur in the same clock cycle */
  reg->RES = (reg->RES & ~RES_OUTPUT_MASK(pwm->channel))
      | RES_OUTPUT(pwm->channel, OUTPUT_CLEAR);
  /* Disable modulation by default */
  reg->OUT[pwm->channel].CLR = 1 << unit->event;
  reg->OUT[pwm->channel].SET = 0;

  /* Enable allocated event in state 0 */
  reg->EV[pwm->event].STATE = 0x00000001;

  return E_OK;
}
/*----------------------------------------------------------------------------*/
#ifndef CONFIG_PLATFORM_NXP_SCT_NO_DEINIT
static void singleEdgeDeinit(void *object)
{
  struct SctPwm * const pwm = object;
  struct SctPwmUnit * const unit = pwm->unit;
  LPC_SCT_Type * const reg = unit->base.reg;

  /* Disable output */
  reg->OUT[pwm->channel].SET = 0;
  reg->OUT[pwm->channel].CLR = 0;
  reg->RES &= ~RES_OUTPUT_MASK(pwm->channel);

  /* Release allocated event */
  reg->EV[pwm->event].CTRL = 0;
  reg->EV[pwm->event].STATE = 0;
  sctReleaseEvent(&unit->base, pwm->event);
}
#endif
/*----------------------------------------------------------------------------*/
static void singleEdgeEnable(void *object)
{
  struct SctPwm * const pwm = object;
  struct SctPwmUnit * const unit = pwm->unit;
  LPC_SCT_Type * const reg = unit->base.reg;

  reg->OUT[pwm->channel].CLR = 1 << pwm->event;
  reg->OUT[pwm->channel].SET = 1 << unit->event;
}
/*----------------------------------------------------------------------------*/
static void singleEdgeDisable(void *object)
{
  struct SctPwm * const pwm = object;
  struct SctPwmUnit * const unit = pwm->unit;
  LPC_SCT_Type * const reg = unit->base.reg;

  /* Clear synchronously */
  reg->OUT[pwm->channel].CLR = 1 << unit->event;
  reg->OUT[pwm->channel].SET = 0;
}
/*----------------------------------------------------------------------------*/
static uint32_t singleEdgeGetResolution(const void *object)
{
  return ((const struct SctPwm *)object)->unit->resolution;
}
/*----------------------------------------------------------------------------*/
static void singleEdgeSetDuration(void *object, uint32_t duration)
{
  struct SctPwm * const pwm = object;
  const uint32_t resolution = pwm->unit->resolution;

  if (duration >= resolution)
    duration = resolution + 1;
  else if (!duration)
    duration = resolution;

  *(volatile uint16_t *)pwm->value = duration - 1;
}
/*----------------------------------------------------------------------------*/
static void singleEdgeSetDurationUnified(void *object, uint32_t duration)
{
  struct SctPwm * const pwm = object;
  const uint32_t resolution = pwm->unit->resolution;

  if (duration >= resolution)
    duration = resolution + 1;
  else if (!duration)
    duration = resolution;

  *(volatile uint32_t *)pwm->value = duration - 1;
}
/*----------------------------------------------------------------------------*/
static void singleEdgeSetEdges(void *object,
    uint32_t leading __attribute__((unused)), uint32_t trailing)
{
  assert(leading == 0); /* Leading edge time must be zero */
  singleEdgeSetDuration(object, trailing);
}
/*----------------------------------------------------------------------------*/
static void singleEdgeSetEdgesUnified(void *object,
    uint32_t leading __attribute__((unused)), uint32_t trailing)
{
  assert(leading == 0); /* Leading edge time must be zero */
  singleEdgeSetDurationUnified(object, trailing);
}
/*----------------------------------------------------------------------------*/
static void singleEdgeSetFrequency(void *object, uint32_t frequency)
{
  struct SctPwm * const pwm = object;
  struct SctPwmUnit * const unit = pwm->unit;

  unit->frequency = frequency;
  sctSetFrequency(&unit->base, unit->frequency * unit->resolution);
}
/*----------------------------------------------------------------------------*/
static enum Result doubleEdgeInit(void *object, const void *configBase)
{
  const struct SctPwmDoubleEdgeConfig * const config = configBase;
  assert(config);

  struct SctPwmDoubleEdge * const pwm = object;
  struct SctPwmUnit * const unit = config->parent;

  /* Allocate events */
  if (!sctAllocateEvent(&unit->base, &pwm->leadingEvent))
    return E_BUSY;
  if (!sctAllocateEvent(&unit->base, &pwm->trailingEvent))
    return E_BUSY;

  /* Initialize output pin */
  pwm->channel = configOutputPin(unit->base.channel, config->pin);
  pwm->unit = unit;

  LPC_SCT_Type * const reg = unit->base.reg;

  if (unit->base.part == SCT_UNIFIED)
  {
    pwm->leading = &reg->MATCHREL[pwm->leadingEvent];
    pwm->trailing = &reg->MATCHREL[pwm->trailingEvent];
  }
  else
  {
    pwm->leading = &reg->MATCHREL_PART[pwm->leadingEvent][unit->base.part];
    pwm->trailing = &reg->MATCHREL_PART[pwm->trailingEvent][unit->base.part];
  }

  /* Configure event */
  const uint32_t controlValue = EVCTRL_OUTSEL | EVCTRL_IOSEL(pwm->channel)
      | EVCTRL_COMBMODE(COMBMODE_MATCH)
      | EVCTRL_DIRECTION(DIRECTION_INDEPENDENT);

  reg->EV[pwm->leadingEvent].CTRL = EVCTRL_MATCHSEL(pwm->leadingEvent)
      | controlValue;
  reg->EV[pwm->trailingEvent].CTRL = EVCTRL_MATCHSEL(pwm->trailingEvent)
      | controlValue;

  if (unit->base.part == SCT_HIGH)
  {
    reg->EV[pwm->leadingEvent].CTRL |= EVCTRL_HEVENT;
    reg->EV[pwm->trailingEvent].CTRL |= EVCTRL_HEVENT;
  }

  /* Clear the output when both events occur in the same clock cycle */
  reg->RES = (reg->RES & ~RES_OUTPUT_MASK(pwm->channel))
      | RES_OUTPUT(pwm->channel, OUTPUT_CLEAR);
  /* Disable modulation by default */
  reg->OUT[pwm->channel].CLR = 1 << unit->event;
  reg->OUT[pwm->channel].SET = 0;

  /* Enable allocated events in state 0 */
  reg->EV[pwm->leadingEvent].STATE = 0x00000001;
  reg->EV[pwm->trailingEvent].STATE = 0x00000001;

  return E_OK;
}
/*----------------------------------------------------------------------------*/
#ifndef CONFIG_PLATFORM_NXP_SCT_NO_DEINIT
static void doubleEdgeDeinit(void *object)
{
  struct SctPwmDoubleEdge * const pwm = object;
  struct SctPwmUnit * const unit = pwm->unit;
  LPC_SCT_Type * const reg = unit->base.reg;

  /* Disable output */
  reg->OUT[pwm->channel].SET = 0;
  reg->OUT[pwm->channel].CLR = 0;
  reg->RES &= ~RES_OUTPUT_MASK(pwm->channel);

  /* Release allocated events */
  reg->EV[pwm->trailingEvent].CTRL = 0;
  reg->EV[pwm->leadingEvent].STATE = 0;
  sctReleaseEvent(&unit->base, pwm->trailingEvent);
  sctReleaseEvent(&unit->base, pwm->leadingEvent);
}
#endif
/*----------------------------------------------------------------------------*/
static void doubleEdgeEnable(void *object)
{
  struct SctPwmDoubleEdge * const pwm = object;
  struct SctPwmUnit * const unit = pwm->unit;
  LPC_SCT_Type * const reg = unit->base.reg;

  reg->OUT[pwm->channel].CLR = 1 << pwm->trailingEvent;
  reg->OUT[pwm->channel].SET = 1 << pwm->leadingEvent;
}
/*----------------------------------------------------------------------------*/
static void doubleEdgeDisable(void *object)
{
  struct SctPwmDoubleEdge * const pwm = object;
  struct SctPwmUnit * const unit = pwm->unit;
  LPC_SCT_Type * const reg = unit->base.reg;

  /* Clear synchronously */
  reg->OUT[pwm->channel].CLR = 1 << unit->event;
  reg->OUT[pwm->channel].SET = 0;
}
/*----------------------------------------------------------------------------*/
static uint32_t doubleEdgeGetResolution(const void *object)
{
  return ((const struct SctPwmDoubleEdge *)object)->unit->resolution;
}
/*----------------------------------------------------------------------------*/
static void doubleEdgeSetDuration(void *object, uint32_t duration)
{
  struct SctPwmDoubleEdge * const pwm = object;
  const uint32_t resolution = pwm->unit->resolution;
  const uint32_t leading = *(const volatile uint16_t *)pwm->leading + 1;
  uint32_t trailing;

  if (duration >= resolution)
  {
    trailing = resolution + 1;
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
static void doubleEdgeSetDurationUnified(void *object, uint32_t duration)
{
  struct SctPwmDoubleEdge * const pwm = object;
  const uint32_t resolution = pwm->unit->resolution;
  const uint32_t leading = *(const volatile uint32_t *)pwm->leading + 1;
  uint32_t trailing;

  if (duration >= resolution)
  {
    trailing = resolution + 1;
  }
  else
  {
    if (leading >= resolution - duration)
      trailing = leading - (resolution - duration);
    else
      trailing = leading + duration;
  }

  doubleEdgeSetEdgesUnified(pwm, leading, trailing);
}
/*----------------------------------------------------------------------------*/
static void doubleEdgeSetEdges(void *object, uint32_t leading,
    uint32_t trailing)
{
  struct SctPwmDoubleEdge * const pwm = object;
  struct SctPwmUnit * const unit = pwm->unit;
  LPC_SCT_Type * const reg = unit->base.reg;
  const uint32_t mask = CONFIG_NORELOAD(unit->base.part == SCT_HIGH);
  const uint32_t resolution = unit->resolution;

  assert(leading < (1UL << 16));
  assert(trailing < (1UL << 16));

  if (!leading)
    leading = resolution;
  if (!trailing)
    trailing = resolution;

  /* Update match reload values atomically by disabling reload */
  reg->CONFIG |= mask;
  *(volatile uint16_t *)pwm->leading = leading - 1;
  *(volatile uint16_t *)pwm->trailing = trailing - 1;
  reg->CONFIG &= ~mask;
}
/*----------------------------------------------------------------------------*/
static void doubleEdgeSetEdgesUnified(void *object, uint32_t leading,
    uint32_t trailing)
{
  struct SctPwmDoubleEdge * const pwm = object;
  struct SctPwmUnit * const unit = pwm->unit;
  LPC_SCT_Type * const reg = unit->base.reg;
  const uint32_t mask = CONFIG_NORELOAD(SCT_LOW);
  const uint32_t resolution = unit->resolution;

  if (!leading)
    leading = resolution;
  if (!trailing)
    trailing = resolution;

  /* Update match reload values atomically by disabling reload */
  reg->CONFIG |= mask;
  *(volatile uint32_t *)pwm->leading = leading - 1;
  *(volatile uint32_t *)pwm->trailing = trailing - 1;
  reg->CONFIG &= ~mask;
}
/*----------------------------------------------------------------------------*/
static void doubleEdgeSetFrequency(void *object, uint32_t frequency)
{
  struct SctPwmDoubleEdge * const pwm = object;
  struct SctPwmUnit * const unit = pwm->unit;

  unit->frequency = frequency;
  sctSetFrequency(&unit->base, unit->frequency * unit->resolution);
}
/*----------------------------------------------------------------------------*/
/**
 * Create single edge PWM channel.
 * @param unit Pointer to an SctPwmUnit object.
 * @param pin Pin used as a signal output.
 * @return Pointer to a new Pwm object on success or zero on error.
 */
void *sctPwmCreate(void *object, PinNumber pin)
{
  struct SctPwmUnit * const unit = object;
  const struct SctPwmConfig channelConfig = {
      .parent = unit,
      .pin = pin
  };

  return init(unit->base.part == SCT_UNIFIED ?
      SctUnifiedPwm : SctPwm, &channelConfig);
}
/*----------------------------------------------------------------------------*/
/**
 * Create double edge PWM channel.
 * @param unit Pointer to a SctPwmUnit object.
 * @param pin Pin used as a signal output.
 * @return Pointer to a new Pwm object on success or zero on error.
 */
void *sctPwmCreateDoubleEdge(void *object, PinNumber pin)
{
  struct SctPwmUnit * const unit = object;
  const struct SctPwmDoubleEdgeConfig channelConfig = {
      .parent = unit,
      .pin = pin
  };

  return init(unit->base.part == SCT_UNIFIED ?
      SctUnifiedPwmDoubleEdge : SctPwmDoubleEdge, &channelConfig);
}
