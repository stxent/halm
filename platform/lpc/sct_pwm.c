/*
 * sct_pwm.c
 * Copyright (C) 2015 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/platform/lpc/sct_defs.h>
#include <halm/platform/lpc/sct_pwm.h>
#include <assert.h>
/*----------------------------------------------------------------------------*/
static void interruptHandler(void *);
static void setUnitResolution(struct SctPwmUnit *, uint8_t, uint32_t);
/*----------------------------------------------------------------------------*/
static enum Result unitInit(void *, const void *);
static void unitEnable(void *);
static void unitDisable(void *);
static void unitSetCallback(void *, void (*)(void *), void *);
static uint32_t unitGetFrequency(const void *);
static void unitSetFrequency(void *, uint32_t);
static uint32_t unitGetOverflow(const void *);
static void unitSetOverflow(void *, uint32_t);

#ifndef CONFIG_PLATFORM_LPC_SCT_NO_DEINIT
static void unitDeinit(void *);
#else
#  define unitDeinit deletedDestructorTrap
#endif
/*----------------------------------------------------------------------------*/
static enum Result singleEdgeInit(void *, const void *);
static void singleEdgeEnable(void *);
static void singleEdgeDisable(void *);
static void singleEdgeSetDuration(void *, uint32_t);
static void singleEdgeSetDurationUnified(void *, uint32_t);
static void singleEdgeSetEdges(void *, uint32_t, uint32_t);
static void singleEdgeSetEdgesUnified(void *, uint32_t, uint32_t);

#ifndef CONFIG_PLATFORM_LPC_SCT_NO_DEINIT
static void singleEdgeDeinit(void *);
#else
#  define singleEdgeDeinit deletedDestructorTrap
#endif
/*----------------------------------------------------------------------------*/
static enum Result doubleEdgeInit(void *, const void *);
static void doubleEdgeEnable(void *);
static void doubleEdgeDisable(void *);
static void doubleEdgeSetDuration(void *, uint32_t);
static void doubleEdgeSetDurationUnified(void *, uint32_t);
static void doubleEdgeSetEdges(void *, uint32_t, uint32_t);
static void doubleEdgeSetEdgesUnified(void *, uint32_t, uint32_t);

#ifndef CONFIG_PLATFORM_LPC_SCT_NO_DEINIT
static void doubleEdgeDeinit(void *);
#else
#  define doubleEdgeDeinit deletedDestructorTrap
#endif
/*----------------------------------------------------------------------------*/
const struct TimerClass * const SctPwmUnit = &(const struct TimerClass){
    .size = sizeof(struct SctPwmUnit),
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

const struct PwmClass * const SctPwm = &(const struct PwmClass){
    .size = sizeof(struct SctPwm),
    .init = singleEdgeInit,
    .deinit = singleEdgeDeinit,

    .enable = singleEdgeEnable,
    .disable = singleEdgeDisable,
    .setDuration = singleEdgeSetDuration,
    .setEdges = singleEdgeSetEdges
};

const struct PwmClass * const SctUnifiedPwm = &(const struct PwmClass){
    .size = sizeof(struct SctPwm),
    .init = singleEdgeInit,
    .deinit = singleEdgeDeinit,

    .enable = singleEdgeEnable,
    .disable = singleEdgeDisable,
    .setDuration = singleEdgeSetDurationUnified,
    .setEdges = singleEdgeSetEdgesUnified
};

const struct PwmClass * const SctPwmDoubleEdge = &(const struct PwmClass){
    .size = sizeof(struct SctPwm),
    .init = doubleEdgeInit,
    .deinit = doubleEdgeDeinit,

    .enable = doubleEdgeEnable,
    .disable = doubleEdgeDisable,
    .setDuration = doubleEdgeSetDuration,
    .setEdges = doubleEdgeSetEdges
};

const struct PwmClass * const SctUnifiedPwmDoubleEdge =
    &(const struct PwmClass){
    .size = sizeof(struct SctPwm),
    .init = doubleEdgeInit,
    .deinit = doubleEdgeDeinit,

    .enable = doubleEdgeEnable,
    .disable = doubleEdgeDisable,
    .setDuration = doubleEdgeSetDurationUnified,
    .setEdges = doubleEdgeSetEdgesUnified
};
/*----------------------------------------------------------------------------*/
static void interruptHandler(void *object)
{
  struct SctPwmUnit * const unit = object;
  unit->callback(unit->callbackArgument);
}
/*----------------------------------------------------------------------------*/
static void setUnitResolution(struct SctPwmUnit *unit, uint8_t channel,
    uint32_t resolution)
{
  const unsigned int part = unit->base.part == SCT_HIGH;
  LPC_SCT_Type * const reg = unit->base.reg;
  const uint32_t match = resolution - 1;

  reg->CONFIG |= CONFIG_NORELOAD(part);

  if (unit->base.part != SCT_UNIFIED)
  {
    assert(match <= 0xFFFF);

    reg->MATCH_PART[channel][part] = match;
    reg->MATCHREL_PART[channel][part] = match;
  }
  else
  {
    /*
     * Max 32-bit value is reserved for the case when a PWM output
     * should stay high during all cycle.
     */
    assert(match < 0xFFFFFFFFUL);

    reg->MATCH[channel] = match;
    reg->MATCHREL[channel] = match;
  }

  reg->CONFIG &= ~CONFIG_NORELOAD(part);
}
/*----------------------------------------------------------------------------*/
static enum Result unitInit(void *object, const void *configBase)
{
  const struct SctPwmUnitConfig * const config = configBase;
  assert(config != NULL);
  assert(config->resolution >= 2);

  const struct SctBaseConfig baseConfig = {
      .channel = config->channel,
      .part = config->part
  };
  struct SctPwmUnit * const unit = object;
  enum Result res;

  /* Call base class constructor */
  if ((res = SctBase->init(unit, &baseConfig)) != E_OK)
    return res;

  if (!sctAllocateEvent(&unit->base, &unit->event))
    return E_BUSY;

  unit->base.handler = interruptHandler;
  unit->callback = NULL;
  unit->frequency = config->frequency;
  unit->resolution = config->resolution;
  unit->input = config->clock;
  unit->centered = config->centered;

  LPC_SCT_Type * const reg = unit->base.reg;
  const uint16_t eventMask = 1 << unit->event;
  const unsigned int part = unit->base.part == SCT_HIGH;

  unit->base.mask = eventMask;
  reg->CTRL_PART[part] = CTRL_HALT;

  if (unit->centered)
    reg->CTRL_PART[part] |= CTRL_BIDIR;

  /* Set base timer frequency */
  sctSetFrequency(&unit->base, unit->frequency);

  /* Automatic reload feature is not supported on all parts */
  reg->CONFIG &= ~(CONFIG_AUTOLIMIT(part) | CONFIG_NORELOAD(part));

  /* Configure timer clock source */
  if (unit->input != SCT_INPUT_NONE)
  {
    reg->CONFIG |= CONFIG_CLKMODE(CLKMODE_INPUT_HP)
        | CONFIG_CKSEL_RISING(unit->input - 1);
  }

  /* Match value should be configured after event initialization */
  setUnitResolution(unit, unit->event, unit->resolution);

  /* Enable match mode for match/capture registers */
  reg->REGMODE_PART[part] &= ~eventMask;

  /* Configure event */
  reg->EV[unit->event].CTRL =
      (unit->base.part == SCT_HIGH ? EVCTRL_HEVENT : 0)
      | EVCTRL_MATCHSEL(unit->event)
      | EVCTRL_COMBMODE(COMBMODE_MATCH)
      | EVCTRL_DIRECTION(DIRECTION_INDEPENDENT);

  /* Reset current state and enable allocated event in state 0 */
  reg->STATE_PART[part] = 0;
  reg->EV[unit->event].STATE = 0x00000001;
  /* Enable timer clearing on allocated event */
  reg->LIMIT_PART[part] = eventMask;

  /* Priority is same for both timer parts */
  irqSetPriority(unit->base.irq, config->priority);

  /* The timer remains disabled */
  return E_OK;
}
/*----------------------------------------------------------------------------*/
#ifndef CONFIG_PLATFORM_LPC_SCT_NO_DEINIT
static void unitDeinit(void *object)
{
  struct SctPwmUnit * const unit = object;
  const unsigned int part = unit->base.part == SCT_HIGH;
  LPC_SCT_Type * const reg = unit->base.reg;

  /* Halt the timer */
  reg->CTRL_PART[part] = CTRL_HALT;
  reg->EVEN &= ~(1 << unit->event);
  reg->LIMIT_PART[part] = 0;

  /* Disable allocated event */
  reg->EV[unit->event].CTRL = 0;
  reg->EV[unit->event].STATE = 0;
  sctReleaseEvent(&unit->base, unit->event);

  SctBase->deinit(unit);
}
#endif
/*----------------------------------------------------------------------------*/
static void unitSetCallback(void *object, void (*callback)(void *),
    void *argument)
{
  struct SctPwmUnit * const unit = object;
  LPC_SCT_Type * const reg = unit->base.reg;
  const uint16_t eventMask = 1 << unit->event;

  unit->callbackArgument = argument;
  unit->callback = callback;

  if (unit->callback != NULL)
  {
    /* Clear pending requests */
    reg->EVFLAG = eventMask;
    /* Enable interrupt requests */
    reg->EVEN |= eventMask;
  }
  else
    reg->EVEN &= ~eventMask;
}
/*----------------------------------------------------------------------------*/
static void unitEnable(void *object)
{
  struct SctPwmUnit * const unit = object;
  LPC_SCT_Type * const reg = unit->base.reg;
  const unsigned int part = unit->base.part == SCT_HIGH;

  reg->EVFLAG = unit->base.mask;
  reg->CTRL_PART[part] &= ~CTRL_HALT;
}
/*----------------------------------------------------------------------------*/
static void unitDisable(void *object)
{
  struct SctPwmUnit * const unit = object;
  LPC_SCT_Type * const reg = unit->base.reg;
  const unsigned int part = unit->base.part == SCT_HIGH;

  reg->CTRL_PART[part] |= CTRL_HALT;
}
/*----------------------------------------------------------------------------*/
static uint32_t unitGetFrequency(const void *object)
{
  const struct SctPwmUnit * const unit = object;
  return unit->frequency;
}
/*----------------------------------------------------------------------------*/
static void unitSetFrequency(void *object, uint32_t frequency)
{
  struct SctPwmUnit * const unit = object;

  unit->frequency = frequency;
  sctSetFrequency(&unit->base, unit->frequency);
}
/*----------------------------------------------------------------------------*/
static uint32_t unitGetOverflow(const void *object)
{
  const struct SctPwmUnit * const unit = object;
  return unit->resolution;
}
/*----------------------------------------------------------------------------*/
static void unitSetOverflow(void *object, uint32_t overflow)
{
  struct SctPwmUnit * const unit = object;

  unit->resolution = overflow;
  setUnitResolution(unit, unit->event, unit->resolution);
}
/*----------------------------------------------------------------------------*/
static enum Result singleEdgeInit(void *object, const void *configBase)
{
  const struct SctPwmConfig * const config = configBase;
  assert(config != NULL);

  struct SctPwm * const pwm = object;
  struct SctPwmUnit * const unit = config->parent;

  if (!sctAllocateEvent(&unit->base, &pwm->event))
    return E_BUSY;

  pwm->channel = sctAllocateOutputChannel(&unit->base, config->pin);
  if (pwm->channel == SCT_OUTPUT_NONE)
    return E_BUSY;

  pwm->unit = unit;
  pwm->inversion = config->inversion;

  const unsigned int pwmIndex = pwm->channel - 1;
  const unsigned int part = unit->base.part == SCT_HIGH;
  LPC_SCT_Type * const reg = unit->base.reg;

  if (unit->base.part == SCT_UNIFIED)
    pwm->value = &reg->MATCHREL[pwm->event];
  else
    pwm->value = &reg->MATCHREL_PART[pwm->event][part];

  /* Enable match mode for match/capture registers */
  reg->REGMODE_PART[part] &= ~(1 << pwm->event);

  /* Configure event */
  reg->EV[pwm->event].CTRL =
      (unit->base.part == SCT_HIGH ? EVCTRL_HEVENT : 0)
      | EVCTRL_MATCHSEL(pwm->event)
      | EVCTRL_COMBMODE(COMBMODE_MATCH)
      | EVCTRL_DIRECTION(DIRECTION_INDEPENDENT);

  /* Configure bidirectional output control */
  uint32_t outputdirctrl =
      reg->OUTPUTDIRCTRL & ~OUTPUTDIRCTRL_SETCLR_MASK(pwmIndex);

  if (unit->centered)
  {
    if (unit->base.part == SCT_HIGH)
      outputdirctrl |= OUTPUTDIRCTRL_SETCLR(pwmIndex, SETCLR_H_REVERSE);
    else
      outputdirctrl |= OUTPUTDIRCTRL_SETCLR(pwmIndex, SETCLR_L_REVERSE);
  }
  reg->OUTPUTDIRCTRL = outputdirctrl;

  /* Set or clear the output when both events occur in the same clock cycle */
  reg->RES = (reg->RES & ~RES_OUTPUT_MASK(pwmIndex))
      | RES_OUTPUT(pwmIndex, pwm->inversion ? OUTPUT_SET : OUTPUT_CLEAR);

  /* Disable modulation by default */
  if (pwm->inversion)
  {
    reg->OUT[pwmIndex].SET = 1 << unit->event;
    reg->OUT[pwmIndex].CLR = 0;
    reg->OUTPUT |= 1 << pwmIndex;
  }
  else
  {
    reg->OUT[pwmIndex].CLR = 1 << unit->event;
    reg->OUT[pwmIndex].SET = 0;
    reg->OUTPUT &= ~(1 << pwmIndex);
  }

  /* Enable allocated event in state 0 */
  reg->EV[pwm->event].STATE = 0x00000001;

  /* Initialize output pin after channel configuration */
  sctConfigOutputPin(&unit->base, pwm->channel, config->pin, pwm->inversion);

  return E_OK;
}
/*----------------------------------------------------------------------------*/
#ifndef CONFIG_PLATFORM_LPC_SCT_NO_DEINIT
static void singleEdgeDeinit(void *object)
{
  struct SctPwm * const pwm = object;
  struct SctPwmUnit * const unit = pwm->unit;
  const unsigned int pwmIndex = pwm->channel - 1;
  LPC_SCT_Type * const reg = unit->base.reg;

  /* Disable allocated event */
  reg->EV[pwm->event].STATE = 0;

  /* Disable the output and overwrite output state to avoid undefined level */
  reg->OUT[pwmIndex].CLR = 0;
  reg->OUT[pwmIndex].SET = 0;
  if (pwm->inversion)
    reg->OUTPUT |= 1 << pwmIndex;
  else
    reg->OUTPUT &= ~(1 << pwmIndex);

  /* Restore default state of other registers */
  reg->OUTPUTDIRCTRL &= ~OUTPUTDIRCTRL_SETCLR_MASK(pwmIndex);
  reg->RES &= ~RES_OUTPUT_MASK(pwmIndex);

  /* Release allocated event */
  reg->EV[pwm->event].CTRL = 0;
  sctReleaseEvent(&unit->base, pwm->event);

  /* Release allocated output */
  sctReleaseOutputChannel(&unit->base, pwm->channel);
}
#endif
/*----------------------------------------------------------------------------*/
static void singleEdgeEnable(void *object)
{
  struct SctPwm * const pwm = object;
  struct SctPwmUnit * const unit = pwm->unit;
  const unsigned int pwmIndex = pwm->channel - 1;
  LPC_SCT_Type * const reg = unit->base.reg;

  if (pwm->inversion)
  {
    reg->OUT[pwmIndex].SET = 1 << pwm->event;
    reg->OUT[pwmIndex].CLR = 1 << unit->event;
  }
  else
  {
    reg->OUT[pwmIndex].CLR = 1 << pwm->event;
    reg->OUT[pwmIndex].SET = 1 << unit->event;
  }
}
/*----------------------------------------------------------------------------*/
static void singleEdgeDisable(void *object)
{
  struct SctPwm * const pwm = object;
  struct SctPwmUnit * const unit = pwm->unit;
  LPC_SCT_Type * const reg = unit->base.reg;
  const uint16_t eventMask = (1 << pwm->event) | (1 << unit->event);
  const unsigned int pwmIndex = pwm->channel - 1;

  /* Set or clear synchronously */
  if (pwm->inversion)
  {
    reg->OUT[pwmIndex].CLR = 0;
    reg->OUT[pwmIndex].SET = eventMask;
  }
  else
  {
    reg->OUT[pwmIndex].SET = 0;
    reg->OUT[pwmIndex].CLR = eventMask;
  }
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
static void singleEdgeSetEdges(void *object, [[maybe_unused]] uint32_t leading,
    uint32_t trailing)
{
  assert(leading == 0); /* Leading edge time must be zero */
  singleEdgeSetDuration(object, trailing);
}
/*----------------------------------------------------------------------------*/
static void singleEdgeSetEdgesUnified(void *object,
    [[maybe_unused]] uint32_t leading, uint32_t trailing)
{
  assert(leading == 0); /* Leading edge time must be zero */
  singleEdgeSetDurationUnified(object, trailing);
}
/*----------------------------------------------------------------------------*/
static enum Result doubleEdgeInit(void *object, const void *configBase)
{
  const struct SctPwmDoubleEdgeConfig * const config = configBase;
  assert(config != NULL);

  struct SctPwmDoubleEdge * const pwm = object;
  struct SctPwmUnit * const unit = config->parent;

  /* Allocate events */
  if (!sctAllocateEvent(&unit->base, &pwm->leadingEvent))
    return E_BUSY;
  if (!sctAllocateEvent(&unit->base, &pwm->trailingEvent))
    return E_BUSY;

  pwm->channel = sctAllocateOutputChannel(&unit->base, config->pin);
  if (pwm->channel == SCT_OUTPUT_NONE)
    return E_BUSY;

  pwm->unit = unit;
  pwm->inversion = config->inversion;

  const unsigned int pwmIndex = pwm->channel - 1;
  const unsigned int part = unit->base.part == SCT_HIGH;
  LPC_SCT_Type * const reg = unit->base.reg;

  if (unit->base.part == SCT_UNIFIED)
  {
    pwm->leading = &reg->MATCHREL[pwm->leadingEvent];
    pwm->trailing = &reg->MATCHREL[pwm->trailingEvent];
  }
  else
  {
    pwm->leading = &reg->MATCHREL_PART[pwm->leadingEvent][part];
    pwm->trailing = &reg->MATCHREL_PART[pwm->trailingEvent][part];
  }

  /* Enable match mode for match/capture registers */
  reg->REGMODE_PART[part] &=
      ~((1 << pwm->leadingEvent) | (1 << pwm->trailingEvent));

  /* Configure event */
  const uint32_t controlValue =
      (unit->base.part == SCT_HIGH ? EVCTRL_HEVENT : 0)
      | EVCTRL_COMBMODE(COMBMODE_MATCH)
      | EVCTRL_DIRECTION(DIRECTION_INDEPENDENT);

  reg->EV[pwm->leadingEvent].CTRL = controlValue
      | EVCTRL_MATCHSEL(pwm->leadingEvent);
  reg->EV[pwm->trailingEvent].CTRL = controlValue
      | EVCTRL_MATCHSEL(pwm->trailingEvent);

  /* Configure bidirectional output control */
  uint32_t outputdirctrl = reg->OUTPUTDIRCTRL
      & ~(OUTPUTDIRCTRL_SETCLR_MASK(pwm->leadingEvent)
          | OUTPUTDIRCTRL_SETCLR_MASK(pwm->trailingEvent));

  if (unit->centered)
  {
    if (unit->base.part == SCT_HIGH)
    {
      outputdirctrl |= OUTPUTDIRCTRL_SETCLR(pwm->leadingEvent, SETCLR_H_REVERSE)
          | OUTPUTDIRCTRL_SETCLR(pwm->trailingEvent, SETCLR_H_REVERSE);
    }
    else
    {
      outputdirctrl |= OUTPUTDIRCTRL_SETCLR(pwm->leadingEvent, SETCLR_L_REVERSE)
          | OUTPUTDIRCTRL_SETCLR(pwm->trailingEvent, SETCLR_L_REVERSE);
    }
  }
  reg->OUTPUTDIRCTRL = outputdirctrl;

  /* Set or clear the output when both events occur in the same clock cycle */
  reg->RES = (reg->RES & ~RES_OUTPUT_MASK(pwmIndex))
      | RES_OUTPUT(pwmIndex, pwm->inversion ? OUTPUT_SET : OUTPUT_CLEAR);

  /* Disable modulation by default */
  if (pwm->inversion)
  {
    reg->OUT[pwmIndex].SET = 1 << unit->event;
    reg->OUT[pwmIndex].CLR = 0;
    reg->OUTPUT |= 1 << pwmIndex;
  }
  else
  {
    reg->OUT[pwmIndex].CLR = 1 << unit->event;
    reg->OUT[pwmIndex].SET = 0;
    reg->OUTPUT &= ~(1 << pwmIndex);
  }

  /* Enable allocated events in state 0 */
  reg->EV[pwm->leadingEvent].STATE = 0x00000001;
  reg->EV[pwm->trailingEvent].STATE = 0x00000001;

  /* Initialize output pin after channel configuration */
  sctConfigOutputPin(&unit->base, pwm->channel, config->pin, pwm->inversion);

  return E_OK;
}
/*----------------------------------------------------------------------------*/
#ifndef CONFIG_PLATFORM_LPC_SCT_NO_DEINIT
static void doubleEdgeDeinit(void *object)
{
  struct SctPwmDoubleEdge * const pwm = object;
  struct SctPwmUnit * const unit = pwm->unit;
  const unsigned int pwmIndex = pwm->channel - 1;
  LPC_SCT_Type * const reg = unit->base.reg;

  /* Disable allocated events */
  reg->EV[pwm->leadingEvent].STATE = 0;
  reg->EV[pwm->trailingEvent].STATE = 0;

  /* Disable the output and overwrite output state to avoid undefined level */
  reg->OUT[pwmIndex].CLR = 0;
  reg->OUT[pwmIndex].SET = 0;
  if (pwm->inversion)
    reg->OUTPUT |= 1 << pwmIndex;
  else
    reg->OUTPUT &= ~(1 << pwmIndex);

  /* Restore default state of other registers */
  reg->OUTPUTDIRCTRL &= ~OUTPUTDIRCTRL_SETCLR_MASK(pwmIndex);
  reg->RES &= ~RES_OUTPUT_MASK(pwmIndex);

  /* Release allocated events */
  reg->EV[pwm->leadingEvent].CTRL = 0;
  reg->EV[pwm->trailingEvent].CTRL = 0;
  sctReleaseEvent(&unit->base, pwm->trailingEvent);
  sctReleaseEvent(&unit->base, pwm->leadingEvent);

  /* Release allocated output */
  sctReleaseOutputChannel(&unit->base, pwm->channel);
}
#endif
/*----------------------------------------------------------------------------*/
static void doubleEdgeEnable(void *object)
{
  struct SctPwmDoubleEdge * const pwm = object;
  struct SctPwmUnit * const unit = pwm->unit;
  const unsigned int pwmIndex = pwm->channel - 1;
  LPC_SCT_Type * const reg = unit->base.reg;

  if (pwm->inversion)
  {
    reg->OUT[pwmIndex].SET = 1 << pwm->trailingEvent;
    reg->OUT[pwmIndex].CLR = 1 << pwm->leadingEvent;
  }
  else
  {
    reg->OUT[pwmIndex].CLR = 1 << pwm->trailingEvent;
    reg->OUT[pwmIndex].SET = 1 << pwm->leadingEvent;
  }
}
/*----------------------------------------------------------------------------*/
static void doubleEdgeDisable(void *object)
{
  struct SctPwmDoubleEdge * const pwm = object;
  struct SctPwmUnit * const unit = pwm->unit;
  LPC_SCT_Type * const reg = unit->base.reg;
  const uint16_t eventMask = (1 << pwm->trailingEvent) | (1 << unit->event);
  const unsigned int pwmIndex = pwm->channel - 1;

  /* Set or clear synchronously */
  if (pwm->inversion)
  {
    reg->OUT[pwmIndex].CLR = 0;
    reg->OUT[pwmIndex].SET = eventMask;
  }
  else
  {
    reg->OUT[pwmIndex].SET = 0;
    reg->OUT[pwmIndex].CLR = eventMask;
  }
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
/**
 * Create single edge PWM channel.
 * @param unit Pointer to an SctPwmUnit object.
 * @param pin Pin used as a signal output.
 * @param inversion Enable output inversion.
 * @return Pointer to a new Pwm object on success or zero on error.
 */
void *sctPwmCreate(void *object, PinNumber pin, bool inversion)
{
  struct SctPwmUnit * const unit = object;
  const struct SctPwmConfig channelConfig = {
      .parent = unit,
      .pin = pin,
      .inversion = inversion
  };

  return init(unit->base.part == SCT_UNIFIED ?
      SctUnifiedPwm : SctPwm, &channelConfig);
}
/*----------------------------------------------------------------------------*/
/**
 * Create double edge PWM channel.
 * @param unit Pointer to a SctPwmUnit object.
 * @param pin Pin used as a signal output.
 * @param inversion Enable output inversion.
 * @return Pointer to a new Pwm object on success or zero on error.
 */
void *sctPwmCreateDoubleEdge(void *object, PinNumber pin, bool inversion)
{
  struct SctPwmUnit * const unit = object;
  const struct SctPwmDoubleEdgeConfig channelConfig = {
      .parent = unit,
      .pin = pin,
      .inversion = inversion
  };

  return init(unit->base.part == SCT_UNIFIED ?
      SctUnifiedPwmDoubleEdge : SctPwmDoubleEdge, &channelConfig);
}
