/*
 * gptimer_capture.c
 * Copyright (C) 2014 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <assert.h>
#include <memory.h>
#include <platform/nxp/gptimer_capture.h>
#include <platform/nxp/gptimer_defs.h>
/*----------------------------------------------------------------------------*/
static void interruptHandler(void *);
/*----------------------------------------------------------------------------*/
static enum result unitSetDescriptor(struct GpTimerCaptureUnit *, uint8_t,
    const struct GpTimerCapture *, struct GpTimerCapture *);
static enum result unitInit(void *, const void *);
static void unitDeinit(void *);
/*----------------------------------------------------------------------------*/
static enum result channelInit(void *, const void *);
static void channelDeinit(void *);
static void channelCallback(void *, void (*)(void *), void *);
static void channelSetEnabled(void *, bool);
static uint32_t channelValue(const void *);
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

    .callback = channelCallback,
    .setEnabled = channelSetEnabled,
    .value = channelValue
};
/*----------------------------------------------------------------------------*/
const struct EntityClass * const GpTimerCaptureUnit = &unitTable;
const struct CaptureClass * const GpTimerCapture = &channelTable;
/*----------------------------------------------------------------------------*/
static void interruptHandler(void *object)
{
  struct GpTimerCaptureUnit * const unit = object;
  LPC_TIMER_Type * const reg = unit->parent.reg;
  const uint32_t state = reg->IR;

  /* Clear pending interrupts */
  reg->IR = state;

  for (uint8_t index = 0; index < ARRAY_SIZE(unit->descriptors); ++index)
  {
    struct GpTimerCapture * const descriptor = unit->descriptors[index];

    if ((state & IR_CAPTURE_INTERRUPT(index)) && descriptor
        && descriptor->callback)
    {
      descriptor->callback(descriptor->callbackArgument);
    }
  }
}
/*----------------------------------------------------------------------------*/
static enum result unitSetDescriptor(struct GpTimerCaptureUnit *unit,
    uint8_t channel, const struct GpTimerCapture *state,
    struct GpTimerCapture *capture)
{
  assert(channel < ARRAY_SIZE(unit->descriptors));

  return compareExchangePointer((void **)(unit->descriptors + channel), state,
      capture) ? E_OK : E_BUSY;
}
/*----------------------------------------------------------------------------*/
static enum result unitInit(void *object, const void *configBase)
{
  const struct GpTimerCaptureUnitConfig * const config = configBase;
  const struct GpTimerBaseConfig parentConfig = {
      .channel = config->channel
  };
  struct GpTimerCaptureUnit * const unit = object;
  enum result res;

  const uint32_t clockFrequency = gpTimerGetClock(object);
  const uint32_t timerFrequency = config->frequency ? config->frequency
      : clockFrequency;

  /* Call base class constructor */
  if ((res = GpTimerBase->init(object, &parentConfig)) != E_OK)
    return res;

  unit->parent.handler = interruptHandler;

  for (uint8_t index = 0; index < ARRAY_SIZE(unit->descriptors); ++index)
    unit->descriptors[index] = 0;

  LPC_TIMER_Type * const reg = unit->parent.reg;

  reg->TCR = 0;

  reg->IR = reg->IR; /* Clear pending interrupts */
  reg->PC = reg->TC = 0;
  reg->CTCR = 0;
  reg->CCR = 0;
  reg->EMR = 0;
  reg->MCR = 0;
  reg->PR = clockFrequency / timerFrequency - 1; /* Configure frequency */

  reg->TCR = TCR_CEN; /* Enable peripheral */

  irqSetPriority(unit->parent.irq, config->priority);
  irqEnable(unit->parent.irq);

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static void unitDeinit(void *object)
{
  struct GpTimerCaptureUnit * const unit = object;
  LPC_TIMER_Type * const reg = unit->parent.reg;

  irqDisable(unit->parent.irq);
  reg->TCR = 0;
  GpTimerBase->deinit(object);
}
/*----------------------------------------------------------------------------*/
static enum result channelInit(void *object, const void *configBase)
{
  const struct GpTimerCaptureConfig * const config = configBase;
  struct GpTimerCapture * const capture = object;
  struct GpTimerCaptureUnit * const unit = config->parent;
  enum result res;

  /* Initialize output pin */
  const int8_t channel = gpTimerConfigCapturePin(unit->parent.channel,
      config->pin);

  if (channel == -1)
    return E_VALUE;

  /* Register object */
  if ((res = unitSetDescriptor(unit, (uint8_t)channel, 0, capture)) != E_OK)
    return res;

  capture->callback = 0;
  capture->channel = (uint8_t)channel;
  capture->event = config->event;
  capture->unit = unit;

  const LPC_TIMER_Type * const reg = capture->unit->parent.reg;

  /* Calculate pointer to capture register for fast access */
  capture->value = reg->CR + capture->channel;
  /*
   * Configure capture events. Function should be called when
   * the initialization of the current object is completed.
   */
  channelSetEnabled(capture, true);

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static void channelDeinit(void *object)
{
  struct GpTimerCapture * const capture = object;

  channelSetEnabled(object, false);
  unitSetDescriptor(capture->unit, capture->channel, capture, 0);
}
/*----------------------------------------------------------------------------*/
static void channelCallback(void *object, void (*callback)(void *),
    void *argument)
{
  struct GpTimerCapture * const capture = object;
  LPC_TIMER_Type * const reg = capture->unit->parent.reg;

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
static void channelSetEnabled(void *object, bool state)
{
  struct GpTimerCapture * const capture = object;
  LPC_TIMER_Type * const reg = capture->unit->parent.reg;

  if (state)
  {
    uint32_t value = 0;

    if (capture->event != PIN_RISING)
      value |= CCR_FALLING_EDGE(capture->channel);
    if (capture->event != PIN_FALLING)
      value |= CCR_RISING_EDGE(capture->channel);

    reg->CCR = (reg->CCR & ~CCR_MASK(capture->channel)) | value;
  }
  else
    reg->CCR &= ~CCR_MASK(capture->channel);
}
/*----------------------------------------------------------------------------*/
static uint32_t channelValue(const void *object)
{
  const struct GpTimerCapture * const capture = object;

  return *capture->value;
}