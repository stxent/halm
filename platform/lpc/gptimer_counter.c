/*
 * gptimer_counter.c
 * Copyright (C) 2016 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/platform/lpc/gptimer_counter.h>
#include <halm/platform/lpc/gptimer_defs.h>
#include <assert.h>
/*----------------------------------------------------------------------------*/
static inline uint32_t getMaxValue(const struct GpTimerCounter *);
static void interruptHandler(void *);
/*----------------------------------------------------------------------------*/
static enum Result tmrInit(void *, const void *);
static void tmrEnable(void *);
static void tmrDisable(void *);
static void tmrSetAutostop(void *, bool);
static void tmrSetCallback(void *, void (*)(void *), void *);
static uint32_t tmrGetOverflow(const void *);
static void tmrSetOverflow(void *, uint32_t);
static uint32_t tmrGetValue(const void *);
static void tmrSetValue(void *, uint32_t);

#ifndef CONFIG_PLATFORM_LPC_GPTIMER_NO_DEINIT
static void tmrDeinit(void *);
#else
#define tmrDeinit deletedDestructorTrap
#endif
/*----------------------------------------------------------------------------*/
const struct TimerClass * const GpTimerCounter = &(const struct TimerClass){
    .size = sizeof(struct GpTimerCounter),
    .init = tmrInit,
    .deinit = tmrDeinit,

    .enable = tmrEnable,
    .disable = tmrDisable,
    .setAutostop = tmrSetAutostop,
    .setCallback = tmrSetCallback,
    .getFrequency = 0,
    .setFrequency = 0,
    .getOverflow = tmrGetOverflow,
    .setOverflow = tmrSetOverflow,
    .getValue = tmrGetValue,
    .setValue = tmrSetValue
};
/*----------------------------------------------------------------------------*/
static inline uint32_t getMaxValue(const struct GpTimerCounter *timer)
{
  return MASK(timer->base.resolution);
}
/*----------------------------------------------------------------------------*/
static void interruptHandler(void *object)
{
  struct GpTimerCounter * const timer = object;
  LPC_TIMER_Type * const reg = timer->base.reg;

  /* Clear all pending interrupts */
  reg->IR = IR_MATCH_MASK | IR_CAPTURE_MASK;

  timer->callback(timer->callbackArgument);
}
/*----------------------------------------------------------------------------*/
static enum Result tmrInit(void *object, const void *configBase)
{
  const struct GpTimerCounterConfig * const config = configBase;
  assert(config);
  assert(config->edge != PIN_HIGH && config->edge != PIN_LOW);

  const struct GpTimerBaseConfig baseConfig = {
      .channel = config->channel
  };
  struct GpTimerCounter * const timer = object;
  enum Result res;

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

    default:
      captureControlValue |= MODE_TOGGLE;
      break;
  }

  /* Call base class constructor */
  if ((res = GpTimerBase->init(timer, &baseConfig)) != E_OK)
    return res;

  timer->base.handler = interruptHandler;
  timer->callback = 0;
  timer->event = (config->event ? config->event : GPTIMER_MATCH0) - 1;

  /* Initialize peripheral block */
  LPC_TIMER_Type * const reg = timer->base.reg;

  reg->TCR = TCR_CRES;
  reg->CCR = 0;
  reg->CTCR = captureControlValue;
  reg->PC = 0;

  /* Clear all pending interrupts */
  reg->IR = IR_MATCH_MASK | IR_CAPTURE_MASK;

  /* Set all match registers to zero for DMA event generation */
  for (size_t i = 0; i < ARRAY_SIZE(reg->MR); ++i)
    reg->MR[i] = 0;

  /* Configure prescaler and default match value */
  reg->MR[timer->event] = getMaxValue(timer);
  /* Reset the timer after reaching the match register value */
  reg->MCR = MCR_RESET(timer->event);
  /* Enable external match to generate signals to other peripherals */
  reg->EMR = EMR_CONTROL(timer->event, CONTROL_TOGGLE);

  /* Clear timer reset flag, but do not enable the timer */
  reg->TCR = 0;

  irqSetPriority(timer->base.irq, config->priority);
  irqEnable(timer->base.irq);

  return E_OK;
}
/*----------------------------------------------------------------------------*/
#ifndef CONFIG_PLATFORM_LPC_GPTIMER_NO_DEINIT
static void tmrDeinit(void *object)
{
  struct GpTimerCounter * const timer = object;
  LPC_TIMER_Type * const reg = timer->base.reg;

  irqDisable(timer->base.irq);
  reg->TCR = 0;

  GpTimerBase->deinit(timer);
}
#endif
/*----------------------------------------------------------------------------*/
static void tmrEnable(void *object)
{
  struct GpTimerCounter * const timer = object;
  LPC_TIMER_Type * const reg = timer->base.reg;

  /* Clear pending interrupt flags and DMA requests */
  reg->IR = IR_MATCH_MASK | IR_CAPTURE_MASK;
  /* Clear external match events to avoid undefined output levels */
  reg->EMR &= ~EMR_EXTERNAL_MATCH_MASK;
  /* Start the timer */
  reg->TCR = TCR_CEN;
}
/*----------------------------------------------------------------------------*/
static void tmrDisable(void *object)
{
  struct GpTimerCounter * const timer = object;
  LPC_TIMER_Type * const reg = timer->base.reg;

  /* Stop the timer */
  reg->TCR = 0;
  /* Clear pending interrupt flags and DMA requests */
  reg->IR = IR_MATCH_MASK | IR_CAPTURE_MASK;
}
/*----------------------------------------------------------------------------*/
static void tmrSetAutostop(void *object, bool state)
{
  struct GpTimerCounter * const timer = object;
  LPC_TIMER_Type * const reg = timer->base.reg;
  const uint32_t mask = MCR_STOP(timer->event);

  if (state)
    reg->MCR |= mask;
  else
    reg->MCR &= ~mask;
}
/*----------------------------------------------------------------------------*/
static void tmrSetCallback(void *object, void (*callback)(void *),
    void *argument)
{
  struct GpTimerCounter * const timer = object;
  LPC_TIMER_Type * const reg = timer->base.reg;
  const uint32_t mask = MCR_INTERRUPT(timer->event);

  timer->callbackArgument = argument;
  timer->callback = callback;

  if (callback)
  {
    reg->IR = IR_MATCH_MASK | IR_CAPTURE_MASK;
    reg->MCR |= mask;
  }
  else
    reg->MCR &= ~mask;
}
/*----------------------------------------------------------------------------*/
static uint32_t tmrGetOverflow(const void *object)
{
  const struct GpTimerCounter * const timer = object;
  const LPC_TIMER_Type * const reg = timer->base.reg;

  return (reg->MR[timer->event] + 1) & getMaxValue(timer);
}
/*----------------------------------------------------------------------------*/
static void tmrSetOverflow(void *object, uint32_t overflow)
{
  struct GpTimerCounter * const timer = object;
  LPC_TIMER_Type * const reg = timer->base.reg;
  const uint32_t value = overflow ? overflow - 1 : getMaxValue(timer);

  assert(value <= getMaxValue(timer));
  reg->MR[timer->event] = value;
}
/*----------------------------------------------------------------------------*/
static uint32_t tmrGetValue(const void *object)
{
  const struct GpTimerCounter * const timer = object;
  const LPC_TIMER_Type * const reg = timer->base.reg;

  return reg->TC;
}
/*----------------------------------------------------------------------------*/
static void tmrSetValue(void *object, uint32_t value)
{
  struct GpTimerCounter * const timer = object;
  LPC_TIMER_Type * const reg = timer->base.reg;

  assert(value <= reg->MR[timer->event]);

  reg->PC = 0;
  reg->TC = value;
}
