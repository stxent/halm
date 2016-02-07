/*
 * sct_pwm.c
 * Copyright (C) 2015 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <assert.h>
#include <platform/nxp/sct_defs.h>
#include <platform/nxp/sct_pwm.h>
#include <platform/platform_defs.h>
/*----------------------------------------------------------------------------*/
#define UNPACK_CHANNEL(value)   (((value) >> 4) & 0x0F)
#define UNPACK_FUNCTION(value)  ((value) & 0x0F)
/*----------------------------------------------------------------------------*/
static uint8_t configOutputPin(uint8_t, pinNumber);
static enum result setMatchValue(struct SctPwmUnit *, uint8_t, uint32_t);
static enum result updateFrequency(struct SctPwmUnit *, uint32_t);
/*----------------------------------------------------------------------------*/
static enum result unitInit(void *, const void *);
static void unitDeinit(void *);
/*----------------------------------------------------------------------------*/
static enum result singleEdgeInit(void *, const void *);
static void singleEdgeDeinit(void *);
static uint32_t singleEdgeGetResolution(const void *);
static void singleEdgeSetDuration(void *, uint32_t);
static void singleEdgeSetEdges(void *, uint32_t, uint32_t);
static void singleEdgeSetEnabled(void *, bool);
static enum result singleEdgeSetFrequency(void *, uint32_t);
/*----------------------------------------------------------------------------*/
static enum result doubleEdgeInit(void *, const void *);
static void doubleEdgeDeinit(void *);
static uint32_t doubleEdgeGetResolution(const void *);
static void doubleEdgeSetDuration(void *, uint32_t);
static void doubleEdgeSetEdges(void *, uint32_t, uint32_t);
static void doubleEdgeSetEnabled(void *, bool);
static enum result doubleEdgeSetFrequency(void *, uint32_t);
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

    .getResolution = singleEdgeGetResolution,
    .setDuration = singleEdgeSetDuration,
    .setEdges = singleEdgeSetEdges,
    .setEnabled = singleEdgeSetEnabled,
    .setFrequency = singleEdgeSetFrequency
};

static const struct PwmClass doubleEdgeTable = {
    .size = sizeof(struct SctPwm),
    .init = doubleEdgeInit,
    .deinit = doubleEdgeDeinit,

    .getResolution = doubleEdgeGetResolution,
    .setDuration = doubleEdgeSetDuration,
    .setEdges = doubleEdgeSetEdges,
    .setEnabled = doubleEdgeSetEnabled,
    .setFrequency = doubleEdgeSetFrequency
};
/*----------------------------------------------------------------------------*/
extern const struct PinEntry sctOutputPins[];
const struct EntityClass * const SctPwmUnit = &unitTable;
const struct PwmClass * const SctPwm = &singleEdgeTable;
const struct PwmClass * const SctPwmDoubleEdge = &doubleEdgeTable;
/*----------------------------------------------------------------------------*/
static uint8_t configOutputPin(uint8_t channel, pinNumber key)
{
  const struct PinEntry * const pinEntry = pinFind(sctOutputPins, key, channel);
  assert(pinEntry);

  const struct Pin pin = pinInit(key);

  pinOutput(pin, 0);
  pinSetFunction(pin, UNPACK_FUNCTION(pinEntry->value));

  return UNPACK_CHANNEL(pinEntry->value);
}
/*----------------------------------------------------------------------------*/
static enum result setMatchValue(struct SctPwmUnit *unit, uint8_t channel,
    uint32_t value)
{
  LPC_SCT_Type * const reg = unit->base.reg;
  const uint8_t offset = unit->base.part == SCT_HIGH;
  enum result res = E_OK;

  reg->CONFIG |= CONFIG_NORELOAD(offset);
  if (unit->base.part != SCT_UNIFIED)
  {
    reg->MATCH[channel] = value;
    reg->MATCHREL[channel] = value;
  }
  else
  {
    if (value < (1 << 16))
    {
      reg->MATCH_PART[channel][offset] = value;
      reg->MATCHREL_PART[channel][offset] = value;
    }
    else
      res = E_VALUE;
  }
  reg->CONFIG &= ~CONFIG_NORELOAD(offset);

  return res;
}
/*----------------------------------------------------------------------------*/
static enum result updateFrequency(struct SctPwmUnit *unit, uint32_t frequency)
{
  LPC_SCT_Type * const reg = unit->base.reg;
  const uint8_t offset = unit->base.part == SCT_HIGH;
  const uint32_t value = (reg->CTRL_PART[offset] & ~CTRL_PRE_MASK)
      | CTRL_CLRCTR;

  if (frequency)
  {
    /* TODO Check whether the clock is from internal source */
    const uint32_t baseClock = sctGetClock((struct SctBase *)unit);
    const uint16_t prescaler = baseClock / frequency - 1;

    if (prescaler >= 256)
      return E_VALUE;

    reg->CTRL_PART[offset] = value | CTRL_PRE(prescaler);
  }
  else
    return E_VALUE;

  unit->frequency = frequency;
  return E_OK;
}
/*----------------------------------------------------------------------------*/
static enum result unitInit(void *object, const void *configBase)
{
  const struct SctPwmUnitConfig * const config = configBase;
  const struct SctBaseConfig baseConfig = {
      .channel = config->channel
  };
  struct SctPwmUnit * const unit = object;
  enum result res;

  /* Call base class constructor */
  if ((res = SctBase->init(object, &baseConfig)) != E_OK)
    return res;

  const int8_t event = sctAllocateEvent((struct SctBase *)unit);

  if (event == -1)
    return E_BUSY;

  unit->event = (uint8_t)event;
  unit->resolution = config->resolution;

  LPC_SCT_Type * const reg = unit->base.reg;
  const uint16_t eventMask = 1 << unit->event;
  const uint8_t offset = unit->base.part == SCT_HIGH;

  unit->base.mask = eventMask;
  reg->CTRL_PART[offset] = CTRL_HALT;

  /* Set desired unit frequency */
  res = updateFrequency(unit, config->frequency * config->resolution);
  if (res != E_OK)
    return res;

  /* Set current match register value */
  reg->CONFIG &= ~(CONFIG_AUTOLIMIT(offset) | CONFIG_NORELOAD(offset));

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
  reg->STATE_PART[offset] = 0;
  reg->EV[unit->event].STATE = 0x00000001;
  /* Enable timer clearing on allocated event */
  reg->LIMIT_PART[offset] = eventMask;

  /* Enable counter */
  reg->CTRL_PART[offset] &= ~CTRL_HALT;

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static void unitDeinit(void *object)
{
  struct SctPwmUnit * const unit = object;
  LPC_SCT_Type * const reg = unit->base.reg;
  const uint8_t offset = unit->base.part == SCT_HIGH;

  /* Halt the timer */
  reg->CTRL_PART[offset] = CTRL_HALT;
  reg->LIMIT_PART[offset] = 0;

  /* Disable allocated event */
  reg->EV[unit->event].CTRL = 0;
  reg->EV[unit->event].STATE = 0;
  sctReleaseEvent((struct SctBase *)unit, unit->event);

  SctBase->deinit(unit);
}
/*----------------------------------------------------------------------------*/
static enum result singleEdgeInit(void *object, const void *configBase)
{
  const struct SctPwmConfig * const config = configBase;
  struct SctPwm * const pwm = object;
  struct SctPwmUnit * const unit = config->parent;

  const int8_t event = sctAllocateEvent((struct SctBase *)unit);

  if (event == -1)
    return E_BUSY;

  /* Initialize output pin */
  const uint8_t channel = configOutputPin(unit->base.channel, config->pin);

  pwm->channel = channel;
  pwm->event = (uint8_t)event;
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
  singleEdgeSetDuration(pwm, config->duration);

  /* Configure setting and clearing of the output */
  reg->RES = (reg->RES & ~RES_OUTPUT_MASK(channel))
      | RES_OUTPUT(channel, OUTPUT_CLEAR);
  reg->OUT[channel].CLR = 1 << pwm->event;
  reg->OUT[channel].SET = 1 << unit->event;

  /* Enable allocated event in state 0 */
  reg->EV[pwm->event].STATE = 0x00000001;

  return E_OK;
}
/*----------------------------------------------------------------------------*/
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
  sctReleaseEvent((struct SctBase *)unit, pwm->event);
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
    const uint8_t offset = unit->base.part == SCT_HIGH;

    reg->MATCHREL_PART[pwm->event][offset] = duration;
  }
  else
    reg->MATCHREL[pwm->event] = duration;
}
/*----------------------------------------------------------------------------*/
static void singleEdgeSetEdges(void *object,
    uint32_t leading __attribute__((unused)), uint32_t trailing)
{
  struct SctPwm * const pwm = object;
  struct SctPwmUnit * const unit = pwm->unit;
  LPC_SCT_Type * const reg = unit->base.reg;

  assert(!leading); /* Leading edge time is constant in single edge mode */

  if (trailing >= unit->resolution)
  {
    trailing = unit->resolution;
  }
  else
  {
    if (!trailing)
      trailing = unit->resolution;
    --trailing;
  }

  if (unit->base.part != SCT_UNIFIED)
  {
    const uint8_t offset = unit->base.part == SCT_HIGH;

    reg->MATCHREL_PART[pwm->event][offset] = trailing;
  }
  else
    reg->MATCHREL[pwm->event] = trailing;
}
/*----------------------------------------------------------------------------*/
static void singleEdgeSetEnabled(void *object, bool state)
{
  struct SctPwm * const pwm = object;
  struct SctPwmUnit * const unit = pwm->unit;
  LPC_SCT_Type * const reg = unit->base.reg;

  if (state)
  {
    reg->OUT[pwm->channel].CLR = 1 << pwm->event;
    reg->OUT[pwm->channel].SET = 1 << unit->event;
  }
  else
  {
    /* Clear synchronously */
    reg->OUT[pwm->channel].CLR = 1 << unit->event;
    reg->OUT[pwm->channel].SET = 0;
  }
}
/*----------------------------------------------------------------------------*/
static enum result singleEdgeSetFrequency(void *object, uint32_t frequency)
{
  struct SctPwm * const pwm = object;
  struct SctPwmUnit * const unit = pwm->unit;

  return updateFrequency(unit, frequency * unit->resolution);
}
/*----------------------------------------------------------------------------*/
static enum result doubleEdgeInit(void *object, const void *configBase)
{
  const struct SctPwmDoubleEdgeConfig * const config = configBase;
  struct SctPwmDoubleEdge * const pwm = object;
  struct SctPwmUnit * const unit = config->parent;

  /* Allocate events */
  const int8_t leadingEvent = sctAllocateEvent((struct SctBase *)unit);
  if (leadingEvent == -1)
    return E_BUSY;
  const int8_t trailingEvent = sctAllocateEvent((struct SctBase *)unit);
  if (trailingEvent == -1)
    return E_BUSY;

  /* Initialize output pin */
  const uint8_t channel = configOutputPin(unit->base.channel, config->pin);

  pwm->channel = channel;
  pwm->leadingEvent = (uint8_t)leadingEvent;
  pwm->trailingEvent = (uint8_t)trailingEvent;
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

  /* Set default match values */
  doubleEdgeSetEdges(pwm, config->leading, config->trailing);

  /* Configure setting and clearing of the output */
  reg->RES = (reg->RES & ~RES_OUTPUT_MASK(channel))
      | RES_OUTPUT(channel, OUTPUT_CLEAR);
  reg->OUT[channel].CLR = 1 << pwm->trailingEvent;
  reg->OUT[channel].SET = 1 << pwm->leadingEvent;

  /* Enable allocated events in state 0 */
  reg->EV[pwm->leadingEvent].STATE = 0x00000001;
  reg->EV[pwm->trailingEvent].STATE = 0x00000001;

  return E_OK;
}
/*----------------------------------------------------------------------------*/
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
  sctReleaseEvent((struct SctBase *)unit, pwm->trailingEvent);
  sctReleaseEvent((struct SctBase *)unit, pwm->leadingEvent);
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
  const uint32_t resolution = unit->resolution;
  const uint8_t offset = unit->base.part == SCT_HIGH;
  LPC_SCT_Type * const reg = unit->base.reg;

  uint32_t center, leading, trailing;

  if (unit->base.part != SCT_UNIFIED)
  {
    leading = reg->MATCHREL_PART[pwm->leadingEvent][offset];
    trailing = reg->MATCHREL_PART[pwm->trailingEvent][offset];
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
  reg->CONFIG |= CONFIG_NORELOAD(offset);
  if (unit->base.part != SCT_UNIFIED)
  {
    reg->MATCHREL_PART[pwm->leadingEvent][offset] = leading;
    reg->MATCHREL_PART[pwm->trailingEvent][offset] = trailing;
  }
  else
  {
    reg->MATCHREL[pwm->leadingEvent] = leading;
    reg->MATCHREL[pwm->trailingEvent] = trailing;
  }
  reg->CONFIG &= ~CONFIG_NORELOAD(offset);
}
/*----------------------------------------------------------------------------*/
static void doubleEdgeSetEdges(void *object, uint32_t leading,
    uint32_t trailing)
{
  struct SctPwmDoubleEdge * const pwm = object;
  struct SctPwmUnit * const unit = pwm->unit;
  const uint32_t resolution = unit->resolution;
  const uint8_t offset = unit->base.part == SCT_HIGH;
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
    {
      leading = trailing = resolution / 2;
    }
  }

  leading = leading ? leading - 1 : resolution - 1;
  trailing = trailing ? trailing - 1 : resolution - 1;

  reg->CONFIG |= CONFIG_NORELOAD(offset);
  if (unit->base.part != SCT_UNIFIED)
  {
    reg->MATCHREL_PART[pwm->leadingEvent][offset] = leading;
    reg->MATCHREL_PART[pwm->trailingEvent][offset] = trailing;
  }
  else
  {
    reg->MATCHREL[pwm->leadingEvent] = leading;
    reg->MATCHREL[pwm->trailingEvent] = trailing;
  }
  reg->CONFIG &= ~CONFIG_NORELOAD(offset);
}
/*----------------------------------------------------------------------------*/
static void doubleEdgeSetEnabled(void *object, bool state)
{
  struct SctPwmDoubleEdge * const pwm = object;
  struct SctPwmUnit * const unit = pwm->unit;
  LPC_SCT_Type * const reg = unit->base.reg;

  if (state)
  {
    reg->OUT[pwm->channel].CLR = 1 << pwm->trailingEvent;
    reg->OUT[pwm->channel].SET = 1 << pwm->leadingEvent;
  }
  else
  {
    /* Clear synchronously */
    reg->OUT[pwm->channel].CLR = 1 << unit->event;
    reg->OUT[pwm->channel].SET = 0;
  }
}
/*----------------------------------------------------------------------------*/
static enum result doubleEdgeSetFrequency(void *object, uint32_t frequency)
{
  struct SctPwmDoubleEdge * const pwm = object;
  struct SctPwmUnit * const unit = pwm->unit;

  return updateFrequency(unit, frequency * unit->resolution);
}
/*----------------------------------------------------------------------------*/
/**
 * Create single edge PWM channel with inverse polarity.
 * @param unit Pointer to an SctPwmUnit object.
 * @param pin Pin used as an output for pulse width modulated signal.
 * @param duration Initial duration in timer ticks.
 * @return Pointer to a new Pwm object on success or zero on error.
 */
void *sctPwmCreate(void *unit, pinNumber pin, uint32_t duration)
{
  const struct SctPwmConfig channelConfig = {
      .parent = unit,
      .duration = duration,
      .pin = pin
  };

  return init(SctPwm, &channelConfig);
}
/*----------------------------------------------------------------------------*/
/**
 * Create double edge PWM channel.
 * @param unit Pointer to a SctPwmUnit object.
 * @param pin Pin used as an output for pulse width modulated signal.
 * @param leading Time of the leading edge in timer ticks.
 * @param trailing Time of the trailing edge in timer ticks.
 * @return Pointer to a new Pwm object on success or zero on error.
 */
void *sctPwmCreateDoubleEdge(void *unit, pinNumber pin, uint32_t leading,
    uint32_t trailing)
{
  const struct SctPwmDoubleEdgeConfig channelConfig = {
      .parent = unit,
      .leading = leading,
      .trailing = trailing,
      .pin = pin
  };

  return init(SctPwmDoubleEdge, &channelConfig);
}
