/*
 * pwm.c
 * Copyright (C) 2026 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/platform/bouffalo/pwm.h>
#include <halm/platform/bouffalo/pwm_defs.h>
#include <halm/pm.h>
#include <assert.h>
/*----------------------------------------------------------------------------*/
#define MIN_RESOLUTION 2
/*----------------------------------------------------------------------------*/
static void interruptHandler(void *);
static void setChannelFrequency(struct PwmUnit *, uint32_t);
static void setChannelOverflow(struct PwmUnit *, uint32_t);

#ifdef CONFIG_PLATFORM_BOUFFALO_PWM_PM
static void powerStateHandler(void *, enum PmState);
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

#ifndef CONFIG_PLATFORM_BOUFFALO_PWM_NO_DEINIT
static void unitDeinit(void *);
#else
#  define unitDeinit deletedDestructorTrap
#endif
/*----------------------------------------------------------------------------*/
static enum Result channelInit(void *, const void *);
static void channelEnable(void *);
static void channelDisable(void *);
static void channelSetDuration(void *, uint32_t);
static void channelSetEdges(void *, uint32_t, uint32_t);

#ifndef CONFIG_PLATFORM_BOUFFALO_PWM_NO_DEINIT
static void channelDeinit(void *);
#else
#  define channelDeinit deletedDestructorTrap
#endif
/*----------------------------------------------------------------------------*/
static enum Result doubleEdgeInit(void *, const void *);
static void doubleEdgeEnable(void *);
static void doubleEdgeDisable(void *);
static void doubleEdgeSetDuration(void *, uint32_t);
static void doubleEdgeSetEdges(void *, uint32_t, uint32_t);

#ifndef CONFIG_PLATFORM_BOUFFALO_PWM_NO_DEINIT
static void doubleEdgeDeinit(void *);
#else
#  define doubleEdgeDeinit deletedDestructorTrap
#endif
/*----------------------------------------------------------------------------*/
const struct TimerClass * const PwmUnit = &(const struct TimerClass){
    .size = sizeof(struct PwmUnit),
    .init = unitInit,
    .deinit = unitDeinit,

    .enable = unitEnable,
    .disable = unitDisable,
    .setAutostop = nullptr,
    .setCallback = unitSetCallback,
    .getFrequency = unitGetFrequency,
    .setFrequency = unitSetFrequency,
    .getOverflow = unitGetOverflow,
    .setOverflow = unitSetOverflow,
    .getValue = nullptr,
    .setValue = nullptr
};

const struct PwmClass * const PwmSingleEdge = &(const struct PwmClass){
    .size = sizeof(struct PwmSingleEdge),
    .init = channelInit,
    .deinit = channelDeinit,

    .enable = channelEnable,
    .disable = channelDisable,
    .setDuration = channelSetDuration,
    .setEdges = channelSetEdges
};

const struct PwmClass * const PwmDoubleEdge = &(const struct PwmClass){
    .size = sizeof(struct PwmDoubleEdge),
    .init = doubleEdgeInit,
    .deinit = doubleEdgeDeinit,

    .enable = doubleEdgeEnable,
    .disable = doubleEdgeDisable,
    .setDuration = doubleEdgeSetDuration,
    .setEdges = doubleEdgeSetEdges
};
/*----------------------------------------------------------------------------*/
static void interruptHandler(void *object)
{
  struct PwmUnit * const unit = object;

  if (unit->callback != nullptr)
    unit->callback(unit->callbackArgument);
}
/*----------------------------------------------------------------------------*/
#ifdef CONFIG_PLATFORM_BOUFFALO_PWM_PM
static void powerStateHandler(void *object, enum PmState state)
{
  if (state == PM_ACTIVE)
  {
    struct PwmUnit * const unit = object;
    setChannelFrequency(unit, unit->frequency);
  }
}
#endif
/*----------------------------------------------------------------------------*/
static void setChannelFrequency(struct PwmUnit *unit, uint32_t frequency)
{
  BL_PWM_Type * const reg = unit->base.reg;
  BL_PWM_CHANNEL_Type * const channel = &reg->CHANNEL[unit->base.channel];
  const uint32_t clock = pwmGetClock(&unit->base);
  uint32_t divider = (frequency && clock > frequency) ? clock / frequency : 1;

  if (divider >= CLKDIV_CLKDIV_MASK)
    divider = CLKDIV_CLKDIV_MASK;
  channel->CLKDIV = divider;
}
/*----------------------------------------------------------------------------*/
static void setChannelOverflow(struct PwmUnit *unit, uint32_t overflow)
{
  BL_PWM_Type * const reg = unit->base.reg;
  BL_PWM_CHANNEL_Type * const channel = &reg->CHANNEL[unit->base.channel];

  assert(overflow >= MIN_RESOLUTION);
  assert(overflow <= PERIOD_PERIOD_MASK);

  channel->PERIOD = overflow;
}
/*----------------------------------------------------------------------------*/
static enum Result unitInit(void *object, const void *configBase)
{
  const struct PwmUnitConfig * const config = configBase;
  assert(config != nullptr);
  assert(config->resolution >= MIN_RESOLUTION);

  const struct PwmBaseConfig baseConfig = {
      .channel = config->channel
  };
  struct PwmUnit * const unit = object;
  enum Result res;

  /* Call base class constructor */
  if ((res = PwmBase->init(unit, &baseConfig)) != E_OK)
    return res;

  unit->base.handler = interruptHandler;
  unit->callback = nullptr;
  unit->callbackArgument = nullptr;
  unit->frequency = config->frequency;
  unit->resolution = config->resolution;

  BL_PWM_Type * const reg = unit->base.reg;
  BL_PWM_CHANNEL_Type * const channel = &reg->CHANNEL[unit->base.channel];

  channel->CONFIG = CONFIG_STOPEN;
  channel->CONFIG |= CONFIG_CLKSEL(CLKSEL_BCLK);

  setChannelFrequency(unit, unit->frequency);
  setChannelOverflow(unit, unit->resolution);

#ifdef CONFIG_PLATFORM_BOUFFALO_PWM_PM
  if ((res = pmRegister(powerStateHandler, unit)) != E_OK)
    return res;
#endif

  irqSetPriority(unit->base.irq, config->priority);
  irqEnable(unit->base.irq);

  /* The channel counter is left in a disabled state */
  return E_OK;
}
/*----------------------------------------------------------------------------*/
#ifndef CONFIG_PLATFORM_BOUFFALO_PWM_NO_DEINIT
static void unitDeinit(void *object)
{
  struct PwmUnit * const unit = object;

  unitDisable(unit);
  irqDisable(unit->base.irq);

#ifdef CONFIG_PLATFORM_BOUFFALO_PWM_PM
  pmUnregister(unit);
#endif

  PwmBase->deinit(unit);
}
#endif
/*----------------------------------------------------------------------------*/
static void unitEnable(void *object)
{
  struct PwmUnit * const unit = object;
  BL_PWM_Type * const reg = unit->base.reg;
  BL_PWM_CHANNEL_Type * const channel = &reg->CHANNEL[unit->base.channel];

  /* Start the counter from zero */
  channel->CONFIG &= ~CONFIG_STOPEN;
}
/*----------------------------------------------------------------------------*/
static void unitDisable(void *object)
{
  struct PwmUnit * const unit = object;
  BL_PWM_Type * const reg = unit->base.reg;
  BL_PWM_CHANNEL_Type * const channel = &reg->CHANNEL[unit->base.channel];

  /* Stop the counter gracefully */
  channel->CONFIG |= CONFIG_STOPEN | CONFIG_STOPMODE;
}
/*----------------------------------------------------------------------------*/
static void unitSetCallback(void *object, void (*callback)(void *),
    void *argument)
{
  struct PwmUnit * const unit = object;
  BL_PWM_Type * const reg = unit->base.reg;
  BL_PWM_CHANNEL_Type * const channel = &reg->CHANNEL[unit->base.channel];

  unit->callbackArgument = argument;
  unit->callback = callback;

  if (unit->callback != nullptr)
    channel->INTERRUPT = INTERRUPT_INTPECN(1) | INTERRUPT_INTEN;
  else
    channel->INTERRUPT &= ~INTERRUPT_INTEN;
}
/*----------------------------------------------------------------------------*/
static uint32_t unitGetFrequency(const void *object)
{
  const struct PwmUnit * const unit = object;
  return unit->frequency;
}
/*----------------------------------------------------------------------------*/
static void unitSetFrequency(void *object, uint32_t frequency)
{
  struct PwmUnit * const unit = object;

  unit->frequency = frequency;
  setChannelFrequency(unit, unit->frequency);
}
/*----------------------------------------------------------------------------*/
static uint32_t unitGetOverflow(const void *object)
{
  const struct PwmUnit * const unit = object;
  return unit->resolution;
}
/*----------------------------------------------------------------------------*/
static void unitSetOverflow(void *object, uint32_t overflow)
{
  struct PwmUnit * const unit = object;

  unit->resolution = overflow;
  setChannelOverflow(unit, unit->resolution);
}
/*----------------------------------------------------------------------------*/
static enum Result channelInit(void *object, const void *configBase)
{
  const struct PwmSingleEdgeConfig * const config = configBase;
  assert(config != nullptr);

  struct PwmSingleEdge * const channel = object;
  struct PwmUnit * const unit = config->parent;

  if (pwmChannelFromPin(config->pin) != unit->base.channel)
    return E_VALUE;

  channel->unit = unit;
  channel->inversion = config->inversion;

  BL_PWM_Type * const reg = unit->base.reg;
  BL_PWM_CHANNEL_Type * const pwm = &reg->CHANNEL[channel->unit->base.channel];
  uint32_t value = pwm->CONFIG & ~CONFIG_SWFVAL;

  value |= CONFIG_SWMODE;
  if (channel->inversion)
    value |= CONFIG_OUTINV | CONFIG_SWFVAL;
  pwm->CONFIG = value;
  pwm->THRE1 = 0;
  pwm->THRE2 = 0;

  /* Configure the output pin */
  pwmConfigPins(config->pin, channel->inversion);

  return E_OK;
}
/*----------------------------------------------------------------------------*/
#ifndef CONFIG_PLATFORM_BOUFFALO_PWM_NO_DEINIT
static void channelDeinit(void *object)
{
  channelDisable(object);
}
#endif
/*----------------------------------------------------------------------------*/
static void channelEnable(void *object)
{
  struct PwmSingleEdge * const channel = object;
  BL_PWM_Type * const reg = channel->unit->base.reg;
  BL_PWM_CHANNEL_Type * const pwm = &reg->CHANNEL[channel->unit->base.channel];

  pwm->CONFIG &= ~CONFIG_SWMODE;
}
/*----------------------------------------------------------------------------*/
static void channelDisable(void *object)
{
  struct PwmSingleEdge * const channel = object;
  BL_PWM_Type * const reg = channel->unit->base.reg;
  BL_PWM_CHANNEL_Type * const pwm = &reg->CHANNEL[channel->unit->base.channel];

  pwm->CONFIG |= CONFIG_SWMODE;
}
/*----------------------------------------------------------------------------*/
static void channelSetDuration(void *object, uint32_t duration)
{
  struct PwmSingleEdge * const channel = object;
  BL_PWM_Type * const reg = channel->unit->base.reg;
  BL_PWM_CHANNEL_Type * const pwm = &reg->CHANNEL[channel->unit->base.channel];

  if (duration > channel->unit->resolution)
    duration = channel->unit->resolution;

  pwm->THRE2 = duration;
}
/*----------------------------------------------------------------------------*/
static void channelSetEdges(void *object, [[maybe_unused]] uint32_t leading,
    uint32_t trailing)
{
  assert(leading == 0); /* Leading edge time must be zero */
  channelSetDuration(object, trailing);
}
/*----------------------------------------------------------------------------*/
static enum Result doubleEdgeInit(void *object, const void *configBase)
{
  const struct PwmDoubleEdgeConfig * const config = configBase;
  assert(config != nullptr);

  struct PwmDoubleEdge * const channel = object;
  struct PwmUnit * const unit = config->parent;

  if (pwmChannelFromPin(config->pin) != unit->base.channel)
    return E_VALUE;

  channel->unit = unit;
  channel->inversion = config->inversion;

  BL_PWM_Type * const reg = unit->base.reg;
  BL_PWM_CHANNEL_Type * const pwm = &reg->CHANNEL[channel->unit->base.channel];
  uint32_t value = pwm->CONFIG & ~CONFIG_SWFVAL;

  value |= CONFIG_SWMODE;
  if (channel->inversion)
    value |= CONFIG_OUTINV | CONFIG_SWFVAL;
  pwm->CONFIG = value;
  pwm->THRE1 = 0;
  pwm->THRE2 = 0;

  /* Configure the output pin */
  pwmConfigPins(config->pin, channel->inversion);

  return E_OK;
}
/*----------------------------------------------------------------------------*/
#ifndef CONFIG_PLATFORM_BOUFFALO_PWM_NO_DEINIT
static void doubleEdgeDeinit(void *object)
{
  doubleEdgeDisable(object);
}
#endif
/*----------------------------------------------------------------------------*/
static void doubleEdgeEnable(void *object)
{
  struct PwmDoubleEdge * const channel = object;
  BL_PWM_Type * const reg = channel->unit->base.reg;
  BL_PWM_CHANNEL_Type * const pwm = &reg->CHANNEL[channel->unit->base.channel];

  pwm->CONFIG &= ~CONFIG_SWMODE;
}
/*----------------------------------------------------------------------------*/
static void doubleEdgeDisable(void *object)
{
  struct PwmDoubleEdge * const channel = object;
  BL_PWM_Type * const reg = channel->unit->base.reg;
  BL_PWM_CHANNEL_Type * const pwm = &reg->CHANNEL[channel->unit->base.channel];

  pwm->CONFIG |= CONFIG_SWMODE;
}
/*----------------------------------------------------------------------------*/
static void doubleEdgeSetDuration(void *object, uint32_t duration)
{
  struct PwmDoubleEdge * const channel = object;
  BL_PWM_Type * const reg = channel->unit->base.reg;
  BL_PWM_CHANNEL_Type * const pwm = &reg->CHANNEL[channel->unit->base.channel];
  const uint32_t resolution = channel->unit->resolution;
  const uint32_t leading = pwm->THRE1;
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

  doubleEdgeSetEdges(channel, leading, trailing);
}
/*----------------------------------------------------------------------------*/
static void doubleEdgeSetEdges(void *object, uint32_t leading,
    uint32_t trailing)
{
  struct PwmDoubleEdge * const channel = object;
  BL_PWM_Type * const reg = channel->unit->base.reg;
  BL_PWM_CHANNEL_Type * const pwm = &reg->CHANNEL[channel->unit->base.channel];
  bool inversion = channel->inversion;

  assert(leading < (1UL << 16));
  assert(trailing < (1UL << 16));

  if (leading > trailing)
  {
    inversion = !inversion;
    pwm->THRE2 = leading;
    pwm->THRE1 = trailing;
  }
  else
  {
    pwm->THRE1 = leading;
    pwm->THRE2 = trailing;
  }

  if (inversion)
    pwm->CONFIG |= CONFIG_OUTINV;
  else
    pwm->CONFIG &= ~CONFIG_OUTINV;
}
/*----------------------------------------------------------------------------*/
/**
 * Create a single edge PWM channel.
 * @param unit Pointer to a PwmUnit object.
 * @param pin Pin used as the signal output.
 * @return Pointer to a new Pwm object on success or zero on error.
 */
void *pwmCreate(void *unit, PinNumber pin, bool inversion)
{
  const struct PwmSingleEdgeConfig channelConfig = {
      .parent = unit,
      .pin = pin,
      .inversion = inversion
  };

  return init(PwmSingleEdge, &channelConfig);
}
/*----------------------------------------------------------------------------*/
/**
 * Create a double edge PWM channel.
 * @param unit Pointer to a PwmUnit object.
 * @param pin Pin used as the signal output.
 * @return Pointer to a new Pwm object on success or zero on error.
 */
void *pwmCreateDoubleEdge(void *unit, PinNumber pin, bool inversion)
{
  const struct PwmDoubleEdgeConfig channelConfig = {
      .parent = unit,
      .pin = pin,
      .inversion = inversion
  };

  return init(PwmDoubleEdge, &channelConfig);
}
