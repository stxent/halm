/*
 * gptimer_capture.c
 * Copyright (C) 2014 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/platform/lpc/gptimer_capture.h>
#include <halm/platform/lpc/gptimer_defs.h>
#include <halm/pm.h>
#include <xcore/memory.h>
#include <assert.h>
/*----------------------------------------------------------------------------*/
static void interruptHandler(void *);

#ifdef CONFIG_PLATFORM_LPC_GPTIMER_PM
static void powerStateHandler(void *, enum PmState);
#endif
/*----------------------------------------------------------------------------*/
static void unitResetInstance(struct GpTimerCaptureUnit *, uint8_t);
static bool unitSetInstance(struct GpTimerCaptureUnit *, uint8_t,
    struct GpTimerCapture *);
static enum Result unitInit(void *, const void *);

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
const struct EntityClass * const GpTimerCaptureUnit =
    &(const struct EntityClass){
    .size = sizeof(struct GpTimerCaptureUnit),
    .init = unitInit,
    .deinit = unitDeinit
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
static void interruptHandler(void *object)
{
  struct GpTimerCaptureUnit * const unit = object;
  LPC_TIMER_Type * const reg = unit->base.reg;
  uint32_t state = reg->IR;

  reg->IR = state; /* Clear pending interrupts */
  state = IR_CAPTURE_VALUE(state); /* Extract capture channel flags */

  for (size_t index = 0; state; state >>= 1, ++index)
  {
    if (state & 1)
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
static void unitResetInstance(struct GpTimerCaptureUnit *unit, uint8_t channel)
{
  unit->instances[channel] = 0;
}
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
  for (size_t index = 0; index < ARRAY_SIZE(unit->instances); ++index)
    unit->instances[index] = 0;

  LPC_TIMER_Type * const reg = unit->base.reg;

  reg->TCR = TCR_CRES;

  reg->IR = reg->IR; /* Clear pending interrupts */
  reg->CCR = 0;
  reg->CTCR = 0;
  reg->EMR = 0;
  reg->MCR = 0;

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
  unitResetInstance(capture->unit, capture->channel);
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
