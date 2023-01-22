/*
 * gptimer_capture.c
 * Copyright (C) 2014 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/platform/lpc/gptimer_capture.h>
#include <halm/platform/lpc/gptimer_defs.h>
#include <halm/pm.h>
#include <assert.h>
/*----------------------------------------------------------------------------*/
static inline uint32_t getMaxValue(const struct GpTimerCaptureUnit *);
static void interruptHandler(void *);

#ifdef CONFIG_PLATFORM_LPC_GPTIMER_PM
static void powerStateHandler(void *, enum PmState);
#endif
/*----------------------------------------------------------------------------*/
static bool unitSetInstance(struct GpTimerCaptureUnit *, uint8_t,
    struct GpTimerCapture *);

static enum Result unitInit(void *, const void *);
static void unitEnable(void *);
static void unitDisable(void *);
static void unitSetCallback(void *, void (*)(void *), void *);
static uint32_t unitGetFrequency(const void *);
static void unitSetFrequency(void *, uint32_t);
static uint32_t unitGetValue(const void *);
static void unitSetValue(void *, uint32_t);

#ifndef CONFIG_PLATFORM_LPC_GPTIMER_NO_DEINIT
static void unitDeinit(void *);
#else
#define unitDeinit deletedDestructorTrap
#endif
/*----------------------------------------------------------------------------*/
static enum Result channelInit(void *, const void *);
static void channelEnable(void *);
static void channelDisable(void *);
static void channelSetCallback(void *, void (*)(void *), void *);
static uint32_t channelGetValue(const void *);

#ifndef CONFIG_PLATFORM_LPC_GPTIMER_NO_DEINIT
static void channelDeinit(void *);
#else
#define channelDeinit deletedDestructorTrap
#endif
/*----------------------------------------------------------------------------*/
const struct TimerClass * const GpTimerCaptureUnit =
    &(const struct TimerClass){
    .size = sizeof(struct GpTimerCaptureUnit),
    .init = unitInit,
    .deinit = unitDeinit,

    .enable = unitEnable,
    .disable = unitDisable,
    .setAutostop = 0,
    .setCallback = unitSetCallback,
    .getFrequency = unitGetFrequency,
    .setFrequency = unitSetFrequency,
    .getOverflow = 0,
    .setOverflow = 0,
    .getValue = unitGetValue,
    .setValue = unitSetValue
};
/*----------------------------------------------------------------------------*/
const struct CaptureClass * const GpTimerCapture =
    &(const struct CaptureClass){
    .size = sizeof(struct GpTimerCapture),
    .init = channelInit,
    .deinit = channelDeinit,

    .enable = channelEnable,
    .disable = channelDisable,
    .setCallback = channelSetCallback,
    .getValue = channelGetValue
};
/*----------------------------------------------------------------------------*/
static inline uint32_t getMaxValue(const struct GpTimerCaptureUnit *timer)
{
  return MASK(timer->base.resolution);
}
/*----------------------------------------------------------------------------*/
static void interruptHandler(void *object)
{
  struct GpTimerCaptureUnit * const unit = object;
  LPC_TIMER_Type * const reg = unit->base.reg;
  const uint32_t state = reg->IR;

  reg->IR = state; /* Clear pending interrupts */

  if (state & IR_MATCH_INTERRUPT(0))
  {
    unit->callback(unit->callbackArgument);
  }

  /* Extract capture channel flags */
  uint32_t capture = IR_CAPTURE_VALUE(state);

  for (size_t index = 0; capture; capture >>= 1, ++index)
  {
    if (capture & 1)
    {
      struct GpTimerCapture * const instance = unit->instances[index];
      instance->callback(instance->callbackArgument);
    }
  }
}
/*----------------------------------------------------------------------------*/
#ifdef CONFIG_PLATFORM_LPC_GPTIMER_PM
static void powerStateHandler(void *object, enum PmState state)
{
  if (state == PM_ACTIVE)
  {
    struct GpTimerCaptureUnit * const unit = object;
    gpTimerSetFrequency(&unit->base, unit->frequency);
  }
}
#endif
/*----------------------------------------------------------------------------*/
static bool unitSetInstance(struct GpTimerCaptureUnit *unit,
    uint8_t channel, struct GpTimerCapture *capture)
{
  assert(channel < ARRAY_SIZE(unit->instances));

  if (!unit->instances[channel])
  {
    unit->instances[channel] = capture;
    return true;
  }
  else
    return false;
}
/*----------------------------------------------------------------------------*/
static enum Result unitInit(void *object, const void *configBase)
{
  const struct GpTimerCaptureUnitConfig * const config = configBase;
  const struct GpTimerBaseConfig baseConfig = {
      .channel = config->channel
  };
  struct GpTimerCaptureUnit * const unit = object;
  enum Result res;

  /* Call base class constructor */
  if ((res = GpTimerBase->init(unit, &baseConfig)) != E_OK)
    return res;

  unit->base.handler = interruptHandler;
  unit->callback = 0;
  for (size_t index = 0; index < ARRAY_SIZE(unit->instances); ++index)
    unit->instances[index] = 0;

  LPC_TIMER_Type * const reg = unit->base.reg;

  reg->TCR = TCR_CRES;
  reg->CCR = 0;
  reg->CTCR = 0;
  reg->EMR = 0;
  reg->MCR = 0;

  /* Clear all pending interrupts */
  reg->IR = IR_MATCH_MASK | IR_CAPTURE_MASK;
  /* Configure default match value for update event */
  reg->MR[0] = getMaxValue(unit);

  unit->frequency = config->frequency;
  gpTimerSetFrequency(&unit->base, unit->frequency);

#ifdef CONFIG_PLATFORM_LPC_GPTIMER_PM
  if ((res = pmRegister(powerStateHandler, unit)) != E_OK)
    return res;
#endif

  /* Start counting */
  reg->TCR = TCR_CEN;

  irqSetPriority(unit->base.irq, config->priority);
  irqEnable(unit->base.irq);

  return res;
}
/*----------------------------------------------------------------------------*/
#ifndef CONFIG_PLATFORM_LPC_GPTIMER_NO_DEINIT
static void unitDeinit(void *object)
{
  struct GpTimerCaptureUnit * const unit = object;
  LPC_TIMER_Type * const reg = unit->base.reg;

  irqDisable(unit->base.irq);
  reg->TCR = 0;

#ifdef CONFIG_PLATFORM_LPC_GPTIMER_PM
  pmUnregister(unit);
#endif

  GpTimerBase->deinit(unit);
}
#endif
/*----------------------------------------------------------------------------*/
static void unitEnable(void *object)
{
  struct GpTimerCaptureUnit * const unit = object;
  LPC_TIMER_Type * const reg = unit->base.reg;

  /* Clear pending interrupt flags and DMA requests */
  reg->IR = IR_MATCH_MASK | IR_CAPTURE_MASK;
  /* Start the timer */
  reg->TCR = TCR_CEN;
}
/*----------------------------------------------------------------------------*/
static void unitDisable(void *object)
{
  struct GpTimerCaptureUnit * const unit = object;
  LPC_TIMER_Type * const reg = unit->base.reg;

  /* Stop the timer */
  reg->TCR = 0;
}
/*----------------------------------------------------------------------------*/
static void unitSetCallback(void *object, void (*callback)(void *),
    void *argument)
{
  struct GpTimerCaptureUnit * const unit = object;
  LPC_TIMER_Type * const reg = unit->base.reg;

  unit->callbackArgument = argument;
  unit->callback = callback;

  if (callback)
  {
    reg->IR = IR_MATCH_MASK;
    reg->MCR |= MCR_INTERRUPT(0);
  }
  else
    reg->MCR &= ~MCR_INTERRUPT(0);
}
/*----------------------------------------------------------------------------*/
static uint32_t unitGetFrequency(const void *object)
{
  const struct GpTimerCaptureUnit * const unit = object;
  const LPC_TIMER_Type * const reg = unit->base.reg;
  const uint32_t baseClock = gpTimerGetClock(&unit->base);

  return baseClock / (reg->PR + 1);
}
/*----------------------------------------------------------------------------*/
static void unitSetFrequency(void *object, uint32_t frequency)
{
  struct GpTimerCaptureUnit * const unit = object;

  unit->frequency = frequency;
  gpTimerSetFrequency(&unit->base, unit->frequency);
}
/*----------------------------------------------------------------------------*/
static uint32_t unitGetValue(const void *object)
{
  const struct GpTimerCaptureUnit * const unit = object;
  const LPC_TIMER_Type * const reg = unit->base.reg;

  return reg->TC;
}
/*----------------------------------------------------------------------------*/
static void unitSetValue(void *object, uint32_t value)
{
  struct GpTimerCaptureUnit * const unit = object;
  LPC_TIMER_Type * const reg = unit->base.reg;

  assert(value <= getMaxValue(unit));

  reg->PC = 0;
  reg->TC = value;
}
/*----------------------------------------------------------------------------*/
static enum Result channelInit(void *object, const void *configBase)
{
  const struct GpTimerCaptureConfig * const config = configBase;
  assert(config);
  assert(config->event != PIN_LOW && config->event != PIN_HIGH);

  struct GpTimerCapture * const capture = object;
  struct GpTimerCaptureUnit * const unit = config->parent;

  /* Initialize output pin */
  capture->channel = gpTimerConfigCapturePin(unit->base.channel,
      config->pin, config->pull);

  /* Register object */
  if (unitSetInstance(unit, capture->channel, capture))
  {
    capture->callback = 0;
    capture->event = config->event;
    capture->unit = unit;

    LPC_TIMER_Type * const reg = capture->unit->base.reg;
    uint32_t captureControlValue = reg->CCR & CCR_MASK(capture->channel);

    capture->value = &reg->CR[capture->channel];

    if (capture->event != PIN_RISING)
      captureControlValue |= CCR_FALLING_EDGE(capture->channel);
    if (capture->event != PIN_FALLING)
      captureControlValue |= CCR_RISING_EDGE(capture->channel);
    reg->CCR = captureControlValue;

    return E_OK;
  }
  else
    return E_BUSY;
}
/*----------------------------------------------------------------------------*/
#ifndef CONFIG_PLATFORM_LPC_GPTIMER_NO_DEINIT
static void channelDeinit(void *object)
{
  struct GpTimerCapture * const capture = object;

  channelDisable(object);
  capture->unit->instances[capture->channel] = 0;
}
#endif
/*----------------------------------------------------------------------------*/
static void channelEnable(void *object)
{
  struct GpTimerCapture * const capture = object;
  LPC_TIMER_Type * const reg = capture->unit->base.reg;

  if (capture->callback)
    reg->CCR |= CCR_INTERRUPT(capture->channel);
}
/*----------------------------------------------------------------------------*/
static void channelDisable(void *object)
{
  struct GpTimerCapture * const capture = object;
  LPC_TIMER_Type * const reg = capture->unit->base.reg;

  reg->CCR &= ~CCR_INTERRUPT(capture->channel);
}
/*----------------------------------------------------------------------------*/
static void channelSetCallback(void *object, void (*callback)(void *),
    void *argument)
{
  struct GpTimerCapture * const capture = object;
  LPC_TIMER_Type * const reg = capture->unit->base.reg;

  capture->callbackArgument = argument;
  capture->callback = callback;

  if (callback)
  {
    reg->IR = IR_CAPTURE_INTERRUPT(capture->channel);
    reg->CCR |= CCR_INTERRUPT(capture->channel);
  }
  else
    reg->CCR &= ~CCR_INTERRUPT(capture->channel);
}
/*----------------------------------------------------------------------------*/
static uint32_t channelGetValue(const void *object)
{
  return *((const struct GpTimerCapture *)object)->value;
}
/*----------------------------------------------------------------------------*/
/**
 * Create capture channel.
 * @param unit Pointer to a GpTimerCaptureUnit object.
 * @param pin Pin used as a signal input.
 * @param event Event type, possible values are @b PIN_RISING, @b PIN_FALLING
 * and @b PIN_TOGGLE.
 * @param pull Pull-up and pull-down control, possible values are @b PIN_NOPULL,
 * @b PIN_PULLUP and @b PIN_PULLDOWN.
 * @return Pointer to a new Capture object on success or zero on error.
 */
void *gpTimerCaptureCreate(void *unit, PinNumber pin, enum PinEvent event,
    enum PinPull pull)
{
  const struct GpTimerCaptureConfig channelConfig = {
      .parent = unit,
      .event = event,
      .pull = pull,
      .pin = pin
  };

  return init(GpTimerCapture, &channelConfig);
}
