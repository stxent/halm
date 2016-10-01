/*
 * gptimer_counter.c
 * Copyright (C) 2016 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <assert.h>
#include <halm/platform/nxp/gptimer_counter.h>
#include <halm/platform/nxp/gptimer_defs.h>
/*----------------------------------------------------------------------------*/
static inline uint32_t getResolution(struct GpTimerCounter *);
static void interruptHandler(void *);
/*----------------------------------------------------------------------------*/
static enum result tmrInit(void *, const void *);
static void tmrDeinit(void *);
static void tmrCallback(void *, void (*)(void *), void *);
static void tmrSetEnabled(void *, bool);
static uint32_t tmrGetFrequency(const void *);
static enum result tmrSetFrequency(void *, uint32_t);
static enum result tmrSetOverflow(void *, uint32_t);
static uint32_t tmrGetValue(const void *);
static enum result tmrSetValue(void *, uint32_t);
/*----------------------------------------------------------------------------*/
static const struct TimerClass tmrTable = {
    .size = sizeof(struct GpTimerCounter),
    .init = tmrInit,
    .deinit = tmrDeinit,

    .callback = tmrCallback,
    .setEnabled = tmrSetEnabled,
    .getFrequency = tmrGetFrequency,
    .setFrequency = tmrSetFrequency,
    .setOverflow = tmrSetOverflow,
    .getValue = tmrGetValue,
    .setValue = tmrSetValue
};
/*----------------------------------------------------------------------------*/
const struct TimerClass * const GpTimerCounter = &tmrTable;
/*----------------------------------------------------------------------------*/
static inline uint32_t getResolution(struct GpTimerCounter *timer)
{
  return MASK(timer->base.resolution);
}
/*----------------------------------------------------------------------------*/
static void interruptHandler(void *object)
{
  struct GpTimerCounter * const timer = object;
  LPC_TIMER_Type * const reg = timer->base.reg;

  if (timer->callback)
    timer->callback(timer->callbackArgument);

  /* Clear all pending interrupts */
  do
  {
    reg->IR = reg->IR;
  }
  while (reg->IR);
}
/*----------------------------------------------------------------------------*/
static enum result tmrInit(void *object, const void *configBase)
{
  const struct GpTimerCounterConfig * const config = configBase;
  const struct GpTimerBaseConfig baseConfig = {
      .channel = config->channel
  };
  struct GpTimerCounter * const timer = object;
  enum result res;

  /* Configure clock input and polarity */
  const uint32_t captureChannel = gpTimerConfigCapturePin(config->channel,
      config->pin, PIN_NOPULL);
  uint32_t captureControlValue = CTCR_INPUT(captureChannel);

  switch (config->edge)
  {
    case PIN_RISING:
      captureControlValue |= MODE_RISING;
      break;

    case PIN_FALLING:
      captureControlValue |= MODE_FALLING;
      break;

    case PIN_TOGGLE:
      captureControlValue |= MODE_TOGGLE;
      break;
  }

  /* Call base class constructor */
  if ((res = GpTimerBase->init(object, &baseConfig)) != E_OK)
    return res;

  timer->base.handler = interruptHandler;
  timer->callback = 0;
  timer->event = GPTIMER_MATCH0 - 1;

  /* Initialize peripheral block */
  LPC_TIMER_Type * const reg = timer->base.reg;

  reg->TCR = TCR_CRES;

  reg->IR = reg->IR; /* Clear pending interrupts */
  reg->CCR = 0;
  reg->CTCR = captureControlValue;
  reg->EMR = 0;
  reg->MCR = 0;
  reg->PC = 0;

  /* Configure prescaler and default match value */
  reg->MR[timer->event] = getResolution(timer);

  /* Timer is disabled by default */
  reg->TCR = 0;

  irqSetPriority(timer->base.irq, config->priority);
  irqEnable(timer->base.irq);

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static void tmrDeinit(void *object)
{
  struct GpTimerCounter * const timer = object;
  LPC_TIMER_Type * const reg = timer->base.reg;

  irqDisable(timer->base.irq);
  reg->TCR = 0;

  GpTimerBase->deinit(timer);
}
/*----------------------------------------------------------------------------*/
static void tmrCallback(void *object, void (*callback)(void *), void *argument)
{
  struct GpTimerCounter * const timer = object;
  LPC_TIMER_Type * const reg = timer->base.reg;

  timer->callbackArgument = argument;
  timer->callback = callback;

  if (callback)
  {
    reg->IR = reg->IR;
    reg->MCR |= MCR_INTERRUPT(timer->event);
  }
  else
    reg->MCR &= ~MCR_INTERRUPT(timer->event);
}
/*----------------------------------------------------------------------------*/
static void tmrSetEnabled(void *object, bool state)
{
  struct GpTimerCounter * const timer = object;
  LPC_TIMER_Type * const reg = timer->base.reg;

  /* Stop the timer and clear pending interrupts */
  reg->TCR = 0;
  reg->IR = IR_MATCH_INTERRUPT(timer->event);

  if (state)
    reg->TCR = TCR_CEN;
}
/*----------------------------------------------------------------------------*/
static uint32_t tmrGetFrequency(const void *object __attribute__((unused)))
{
  /* Current frequency is undefined for this class of timers */
  return 0;
}
/*----------------------------------------------------------------------------*/
static enum result tmrSetFrequency(void *object __attribute__((unused)),
    uint32_t frequency __attribute__((unused)))
{
  return E_INVALID;
}
/*----------------------------------------------------------------------------*/
static enum result tmrSetOverflow(void *object, uint32_t overflow)
{
  struct GpTimerCounter * const timer = object;
  LPC_TIMER_Type * const reg = timer->base.reg;
  const uint32_t resolution = getResolution(timer);
  const bool enabled = reg->TCR & TCR_CEN;

  if (overflow)
  {
    --overflow;

    if (overflow > resolution)
      return E_VALUE;

    reg->TCR = reg->MR[timer->event] >= overflow ? TCR_CRES : 0;

    reg->MR[timer->event] = overflow;
    reg->MCR |= MCR_RESET(timer->event);
  }
  else
  {
    reg->TCR = 0;

    reg->MR[timer->event] = resolution;
    reg->MCR &= ~MCR_RESET(timer->event);
  }

  reg->TCR = enabled ? TCR_CEN : 0;

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static uint32_t tmrGetValue(const void *object)
{
  const struct GpTimerCounter * const timer = object;
  const LPC_TIMER_Type * const reg = timer->base.reg;

  return reg->TC;
}
/*----------------------------------------------------------------------------*/
static enum result tmrSetValue(void *object, uint32_t value)
{
  struct GpTimerCounter * const timer = object;
  LPC_TIMER_Type * const reg = timer->base.reg;
  const bool enabled = reg->TCR & TCR_CEN;

  if (value <= reg->MR[timer->event])
  {
    reg->TCR = 0;

    reg->PC = 0;
    reg->TC = value;

    if (enabled)
      reg->TCR = TCR_CEN;

    return E_OK;
  }
  else
    return E_VALUE;
}
