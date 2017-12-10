/*
 * sct_pwm.c
 * Copyright (C) 2015 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <assert.h>
#include <halm/platform/nxp/sct_defs.h>
#include <halm/platform/nxp/sct_pwm.h>
#include <halm/platform/platform_defs.h>
/*----------------------------------------------------------------------------*/
#define UNPACK_CHANNEL(value)   (((value) >> 4) & 0x0F)
#define UNPACK_FUNCTION(value)  ((value) & 0x0F)
/*----------------------------------------------------------------------------*/
static uint8_t configOutputPin(uint8_t, PinNumber);
static enum Result setMatchValue(struct SctPwmUnit *, uint8_t, uint32_t);
static enum Result updateFrequency(struct SctPwmUnit *, uint32_t);
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
static void singleEdgeSetEdges(void *, uint32_t, uint32_t);
static enum Result singleEdgeSetFrequency(void *, uint32_t);

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
static void doubleEdgeSetEdges(void *, uint32_t, uint32_t);
static enum Result doubleEdgeSetFrequency(void *, uint32_t);

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
/*----------------------------------------------------------------------------*/
extern const struct PinEntry sctOutputPins[];
const struct EntityClass * const SctPwmUnit = &unitTable;
const struct PwmClass * const SctPwm = &singleEdgeTable;
const struct PwmClass * const SctPwmDoubleEdge = &doubleEdgeTable;
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
static enum Result setMatchValue(struct SctPwmUnit *unit, uint8_t channel,
    uint32_t value)
{
  const unsigned int part = unit->base.part == SCT_HIGH;
  LPC_SCT_Type * const reg = unit->base.reg;
  enum Result res = E_OK;

  reg->CONFIG |= CONFIG_NORELOAD(part);
  if (unit->base.part != SCT_UNIFIED)
  {
    if (value < (1 << 16))
    {
      reg->MATCH_PART[channel][part] = value;
      reg->MATCHREL_PART[channel][part] = value;
    }
    else
      res = E_VALUE;
  }
  else
  {
    reg->MATCH[channel] = value;
    reg->MATCHREL[channel] = value;
  }
  reg->CONFIG &= ~CONFIG_NORELOAD(part);

  return res;
}
/*----------------------------------------------------------------------------*/
static enum Result updateFrequency(struct SctPwmUnit *unit, uint32_t frequency)
{
  const unsigned int part = unit->base.part == SCT_HIGH;
  LPC_SCT_Type * const reg = unit->base.reg;

  /* Counter clearing is recommended by the user manual */
  const uint32_t value = (reg->CTRL_PART[part] & ~CTRL_PRE_MASK) | CTRL_CLRCTR;

  if (frequency)
  {
    /* TODO Check whether the clock is from internal source */
    const uint32_t baseClock = sctGetClock(&unit->base);
    const uint16_t prescaler = baseClock / frequency - 1;

    if (prescaler >= 256)
      return E_VALUE;

    reg->CTRL_PART[part] = value | CTRL_PRE(prescaler);
  }
  else
    return E_VALUE;

  unit->frequency = frequency;
  return E_OK;
}
/*----------------------------------------------------------------------------*/
static enum Result unitInit(void *object, const void *configBase)
{
  const struct SctPwmUnitConfig * const config = configBase;
  assert(config);

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

  const int event = sctAllocateEvent(&unit->base);

  if (event == -1)
    return E_BUSY;

  unit->event = event;
  unit->resolution = config->resolution;

  LPC_SCT_Type * const reg = unit->base.reg;
  const unsigned int part = unit->base.part == SCT_HIGH;
  const uint16_t eventMask = 1 << unit->event;

  unit->base.mask = eventMask;
  reg->CTRL_PART[part] = CTRL_HALT;

  /* Set desired unit frequency */
  res = updateFrequency(unit, config->frequency * config->resolution);
  if (res != E_OK)
    return res;

  /* Set current match register value */
  reg->CONFIG &= ~(CONFIG_AUTOLIMIT(part) | CONFIG_NORELOAD(part));

  /* Should be called after event initialization */
  if ((res = setMatchValue(unit, unit->event, unit->resolution - 1)) != E_OK)
    return res;

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

  const int event = sctAllocateEvent(&unit->base);

  if (event == -1)
    return E_BUSY;

  /* Initialize output pin */
  const uint8_t channel = configOutputPin(unit->base.channel, config->pin);

  pwm->channel = channel;
  pwm->event = event;
  pwm->unit = unit;

  LPC_SCT_Type * const reg = unit->base.reg;

  /* Configure event */
  reg->EV[pwm->event].CTRL = EVCTRL_MATCHSEL(pwm->event)
      | EVCTRL_OUTSEL | EVCTRL_IOSEL(channel)
      | EVCTRL_COMBMODE(COMBMODE_MATCH)
      | EVCTRL_DIRECTION(DIRECTION_INDEPENDENT);

  if (unit->base.part == SCT_HIGH)
    reg->EV[pwm->event].CTRL |= EVCTRL_HEVENT;

  /* Set default match value */
  singleEdgeSetDuration(pwm, 0);

  /* Configure setting and clearing of the output */
  reg->RES = (reg->RES & ~RES_OUTPUT_MASK(channel))
      | RES_OUTPUT(channel, OUTPUT_CLEAR);
  /* Disable modulation by default */
  reg->OUT[channel].CLR = 1 << unit->event;
  reg->OUT[channel].SET = 0;

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

  /* Halt the timer */
  reg->OUT[pwm->channel].SET = 0;
  reg->OUT[pwm->channel].CLR = 0;
  reg->RES &= ~RES_OUTPUT_MASK(pwm->channel);

  /* Disable allocated event */
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
  struct SctPwmUnit * const unit = pwm->unit;
  LPC_SCT_Type * const reg = unit->base.reg;

  if (duration >= unit->resolution)
  {
    duration = unit->resolution;
  }
  else
  {
    if (!duration)
      duration = unit->resolution;
    --duration;
  }

  if (unit->base.part != SCT_UNIFIED)
  {
    const unsigned int part = unit->base.part == SCT_HIGH;

    reg->MATCHREL_PART[pwm->event][part] = duration;
  }
  else
    reg->MATCHREL[pwm->event] = duration;
}
/*----------------------------------------------------------------------------*/
static void singleEdgeSetEdges(void *object,
    uint32_t leading __attribute__((unused)), uint32_t trailing)
{
  /* Leading edge time is constant in the single edge mode */
  assert(leading == 0);

  singleEdgeSetDuration(object, trailing);
}
/*----------------------------------------------------------------------------*/
static enum Result singleEdgeSetFrequency(void *object, uint32_t frequency)
{
  struct SctPwm * const pwm = object;
  struct SctPwmUnit * const unit = pwm->unit;

  return updateFrequency(unit, frequency * unit->resolution);
}
/*----------------------------------------------------------------------------*/
static enum Result doubleEdgeInit(void *object, const void *configBase)
{
  const struct SctPwmDoubleEdgeConfig * const config = configBase;
  assert(config);

  struct SctPwmDoubleEdge * const pwm = object;
  struct SctPwmUnit * const unit = config->parent;

  /* Allocate events */
  const int leadingEvent = sctAllocateEvent(&unit->base);
  if (leadingEvent == -1)
    return E_BUSY;

  const int trailingEvent = sctAllocateEvent(&unit->base);
  if (trailingEvent == -1)
    return E_BUSY;

  /* Initialize output pin */
  const uint8_t channel = configOutputPin(unit->base.channel, config->pin);

  pwm->channel = channel;
  pwm->leadingEvent = leadingEvent;
  pwm->trailingEvent = trailingEvent;
  pwm->unit = unit;

  LPC_SCT_Type * const reg = unit->base.reg;

  /* Configure event */
  const uint32_t controlValue = EVCTRL_OUTSEL | EVCTRL_IOSEL(channel)
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

  /* Set initial match values */
  doubleEdgeSetEdges(pwm, 0, 0);

  /* Configure setting and clearing of the output */
  reg->RES = (reg->RES & ~RES_OUTPUT_MASK(channel))
      | RES_OUTPUT(channel, OUTPUT_CLEAR);
  /* Disable modulation by default */
  reg->OUT[channel].CLR = 1 << unit->event;
  reg->OUT[channel].SET = 0;

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

  /* Halt the timer */
  reg->OUT[pwm->channel].SET = 0;
  reg->OUT[pwm->channel].CLR = 0;
  reg->RES &= ~RES_OUTPUT_MASK(pwm->channel);

  /* Disable allocated event */
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
  struct SctPwmUnit * const unit = pwm->unit;
  const unsigned int part = unit->base.part == SCT_HIGH;
  const uint32_t resolution = unit->resolution;
  LPC_SCT_Type * const reg = unit->base.reg;

  uint32_t center, leading, trailing;

  if (unit->base.part != SCT_UNIFIED)
  {
    leading = reg->MATCHREL_PART[pwm->leadingEvent][part];
    trailing = reg->MATCHREL_PART[pwm->trailingEvent][part];
  }
  else
  {
    leading = reg->MATCHREL[pwm->leadingEvent];
    trailing = reg->MATCHREL[pwm->trailingEvent];
  }

  if (leading > trailing)
  {
    const uint32_t half = resolution / 2;

    center = trailing + (leading - trailing) / 2;
    if (center < half)
      center += half;
    else
      center -= half;
  }
  else
  {
    center = leading;
    if (trailing < resolution)
      center += (trailing - leading) / 2;
  }

  if (duration >= resolution)
  {
    leading = center;
    trailing = resolution;
  }
  else
  {
    duration = duration / 2;

    const uint32_t negOffset = center - duration;
    const uint32_t posOffset = center + duration;

    leading = center >= duration ? negOffset : negOffset + resolution;
    trailing = posOffset < resolution ? posOffset : posOffset - resolution;
  }

  /* Update match reload values atomically by disabling reload */
  reg->CONFIG |= CONFIG_NORELOAD(part);
  if (unit->base.part != SCT_UNIFIED)
  {
    reg->MATCHREL_PART[pwm->leadingEvent][part] = leading;
    reg->MATCHREL_PART[pwm->trailingEvent][part] = trailing;
  }
  else
  {
    reg->MATCHREL[pwm->leadingEvent] = leading;
    reg->MATCHREL[pwm->trailingEvent] = trailing;
  }
  reg->CONFIG &= ~CONFIG_NORELOAD(part);
}
/*----------------------------------------------------------------------------*/
static void doubleEdgeSetEdges(void *object, uint32_t leading,
    uint32_t trailing)
{
  struct SctPwmDoubleEdge * const pwm = object;
  struct SctPwmUnit * const unit = pwm->unit;
  const unsigned int part = unit->base.part == SCT_HIGH;
  const uint32_t resolution = unit->resolution;
  LPC_SCT_Type * const reg = unit->base.reg;

  if (leading > resolution)
    leading = resolution;
  if (trailing > resolution)
    trailing = resolution;

  if (trailing >= leading)
  {
    if (trailing - leading >= resolution)
    {
      leading = resolution / 2;
      trailing = resolution + 1;
    }
  }
  else
  {
    if (leading - trailing >= resolution)
      leading = trailing = resolution / 2;
  }

  leading = leading ? leading - 1 : resolution - 1;
  trailing = trailing ? trailing - 1 : resolution - 1;

  reg->CONFIG |= CONFIG_NORELOAD(part);
  if (unit->base.part != SCT_UNIFIED)
  {
    reg->MATCHREL_PART[pwm->leadingEvent][part] = leading;
    reg->MATCHREL_PART[pwm->trailingEvent][part] = trailing;
  }
  else
  {
    reg->MATCHREL[pwm->leadingEvent] = leading;
    reg->MATCHREL[pwm->trailingEvent] = trailing;
  }
  reg->CONFIG &= ~CONFIG_NORELOAD(part);
}
/*----------------------------------------------------------------------------*/
static enum Result doubleEdgeSetFrequency(void *object, uint32_t frequency)
{
  struct SctPwmDoubleEdge * const pwm = object;
  struct SctPwmUnit * const unit = pwm->unit;

  return updateFrequency(unit, frequency * unit->resolution);
}
/*----------------------------------------------------------------------------*/
/**
 * Create single edge PWM channel.
 * @param unit Pointer to an SctPwmUnit object.
 * @param pin Pin used as a signal output.
 * @return Pointer to a new Pwm object on success or zero on error.
 */
void *sctPwmCreate(void *unit, PinNumber pin)
{
  const struct SctPwmConfig channelConfig = {
      .parent = unit,
      .pin = pin
  };

  return init(SctPwm, &channelConfig);
}
/*----------------------------------------------------------------------------*/
/**
 * Create double edge PWM channel.
 * @param unit Pointer to a SctPwmUnit object.
 * @param pin Pin used as a signal output.
 * @return Pointer to a new Pwm object on success or zero on error.
 */
void *sctPwmCreateDoubleEdge(void *unit, PinNumber pin)
{
  const struct SctPwmDoubleEdgeConfig channelConfig = {
      .parent = unit,
      .pin = pin
  };

  return init(SctPwmDoubleEdge, &channelConfig);
}
