/*
 * gptimer_capture.c
 * Copyright (C) 2014 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <assert.h>
#include <xcore/memory.h>
#include <halm/platform/nxp/gptimer_capture.h>
#include <halm/platform/nxp/gptimer_defs.h>
/*----------------------------------------------------------------------------*/
static void interruptHandler(void *);
/*----------------------------------------------------------------------------*/
static enum Result unitSetDescriptor(struct GpTimerCaptureUnit *, uint8_t,
    const struct GpTimerCapture *, struct GpTimerCapture *);
static enum Result unitInit(void *, const void *);
static void unitDeinit(void *);
/*----------------------------------------------------------------------------*/
static enum Result channelInit(void *, const void *);
static void channelDeinit(void *);
static void channelEnable(void *);
static void channelDisable(void *);
static void channelSetCallback(void *, void (*)(void *), void *);
static uint32_t channelGetValue(const void *);
/*----------------------------------------------------------------------------*/
static const struct EntityClass unitTable = {
    .size = sizeof(struct GpTimerCaptureUnit),
    .init = unitInit,
    .deinit = unitDeinit
};
/*----------------------------------------------------------------------------*/
static const struct CaptureClass channelTable = {
    .size = sizeof(struct GpTimerCapture),
    .init = channelInit,
    .deinit = channelDeinit,

    .enable = channelEnable,
    .disable = channelDisable,
    .setCallback = channelSetCallback,
    .getValue = channelGetValue
};
/*----------------------------------------------------------------------------*/
const struct EntityClass * const GpTimerCaptureUnit = &unitTable;
const struct CaptureClass * const GpTimerCapture = &channelTable;
/*----------------------------------------------------------------------------*/
static void interruptHandler(void *object)
{
  struct GpTimerCaptureUnit * const unit = object;
  LPC_TIMER_Type * const reg = unit->base.reg;
  uint32_t state = reg->IR;

  reg->IR = state; /* Clear pending interrupts */
  state = IR_CAPTURE_VALUE(state);

  for (size_t index = 0; state; state >>= 1, ++index)
  {
    if (state & (1 << index))
    {
      struct GpTimerCapture * const descriptor = unit->descriptors[index];

      if (descriptor->callback)
        descriptor->callback(descriptor->callbackArgument);
    }
  }
}
/*----------------------------------------------------------------------------*/
static enum Result unitSetDescriptor(struct GpTimerCaptureUnit *unit,
    uint8_t channel, const struct GpTimerCapture *state,
    struct GpTimerCapture *capture)
{
  if (channel >= ARRAY_SIZE(unit->descriptors))
    return E_VALUE;

  return compareExchangePointer((void **)(unit->descriptors + channel),
      state, capture) ? E_OK : E_BUSY;
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

  const uint32_t clockFrequency = gpTimerGetClock(object);
  const uint32_t timerFrequency = config->frequency ?
      config->frequency : clockFrequency;

  /* Call base class constructor */
  if ((res = GpTimerBase->init(object, &baseConfig)) != E_OK)
    return res;

  unit->base.handler = interruptHandler;

  for (size_t index = 0; index < ARRAY_SIZE(unit->descriptors); ++index)
    unit->descriptors[index] = 0;

  LPC_TIMER_Type * const reg = unit->base.reg;

  reg->TCR = 0;

  reg->IR = reg->IR; /* Clear pending interrupts */
  reg->PC = reg->TC = 0;
  reg->CTCR = 0;
  reg->CCR = 0;
  reg->EMR = 0;
  reg->MCR = 0;
  reg->PR = clockFrequency / timerFrequency - 1; /* Configure frequency */

  reg->TCR = TCR_CEN; /* Enable peripheral */

  irqSetPriority(unit->base.irq, config->priority);
  irqEnable(unit->base.irq);

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static void unitDeinit(void *object)
{
  struct GpTimerCaptureUnit * const unit = object;
  LPC_TIMER_Type * const reg = unit->base.reg;

  irqDisable(unit->base.irq);
  reg->TCR = 0;
  GpTimerBase->deinit(unit);
}
/*----------------------------------------------------------------------------*/
static enum Result channelInit(void *object, const void *configBase)
{
  const struct GpTimerCaptureConfig * const config = configBase;
  assert(config);

  struct GpTimerCapture * const capture = object;
  struct GpTimerCaptureUnit * const unit = config->parent;
  enum Result res;

  /* Initialize output pin */
  capture->channel = gpTimerConfigCapturePin(unit->base.channel,
      config->pin, config->pull);

  /* Register object */
  if ((res = unitSetDescriptor(unit, capture->channel, 0, capture)) != E_OK)
    return res;

  capture->callback = 0;
  capture->event = config->event;
  capture->unit = unit;

  const LPC_TIMER_Type * const reg = capture->unit->base.reg;

  /* Calculate pointer to capture register for fast access */
  capture->value = reg->CR + capture->channel;
  /*
   * Configure capture events. Function should be called when
   * the initialization of the object is completed.
   */
  channelEnable(capture);

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static void channelDeinit(void *object)
{
  struct GpTimerCapture * const capture = object;

  channelDisable(object);
  unitSetDescriptor(capture->unit, capture->channel, capture, 0);
}
/*----------------------------------------------------------------------------*/
static void channelEnable(void *object)
{
  struct GpTimerCapture * const capture = object;
  LPC_TIMER_Type * const reg = capture->unit->base.reg;
  uint32_t value = 0;

  if (capture->event != PIN_RISING)
    value |= CCR_FALLING_EDGE(capture->channel);
  if (capture->event != PIN_FALLING)
    value |= CCR_RISING_EDGE(capture->channel);

  reg->CCR = (reg->CCR & ~CCR_MASK(capture->channel)) | value;
}
/*----------------------------------------------------------------------------*/
static void channelDisable(void *object)
{
  struct GpTimerCapture * const capture = object;
  LPC_TIMER_Type * const reg = capture->unit->base.reg;

  reg->CCR &= ~CCR_MASK(capture->channel);
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
    reg->IR = reg->IR;
    reg->CCR |= CCR_INTERRUPT(capture->channel);
  }
  else
    reg->CCR &= ~CCR_INTERRUPT(capture->channel);
}
/*----------------------------------------------------------------------------*/
static uint32_t channelGetValue(const void *object)
{
  const struct GpTimerCapture * const capture = object;

  return *capture->value;
}
