/*
 * gptimer_counter.c
 * Copyright (C) 2016 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
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
  reg->IR = reg->IR;

  timer->callback(timer->callbackArgument);
}
/*----------------------------------------------------------------------------*/
static enum Result tmrInit(void *object, const void *configBase)
{
  const struct GpTimerCounterConfig * const config = configBase;
  assert(config);

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

    case PIN_TOGGLE:
      captureControlValue |= MODE_TOGGLE;
      break;
  }

  /* Call base class constructor */
  if ((res = GpTimerBase->init(timer, &baseConfig)) != E_OK)
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
  reg->MR[timer->event] = getMaxValue(timer);

  /* Timer is disabled by default */
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

  /* Clear pending interrupts */
  reg->IR = IR_MATCH_INTERRUPT(timer->event);
  /* Start the timer */
  reg->TCR = TCR_CEN;
}
/*----------------------------------------------------------------------------*/
static void tmrDisable(void *object)
{
  struct GpTimerCounter * const timer = object;
  LPC_TIMER_Type * const reg = timer->base.reg;

  reg->TCR = 0;
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
    reg->IR = reg->IR;
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
  const uint32_t mask = MCR_RESET(timer->event);
  const uint32_t resolution = getMaxValue(timer);
  const uint32_t state = reg->TCR & TCR_CEN;

  /* Stop the timer before reconfiguration */
  reg->TCR = 0;

  assert((overflow - 1) <= resolution);
  overflow = (overflow - 1) & resolution;

  /* Setup the match register */
  reg->MR[timer->event] = overflow;

  if (overflow != resolution)
  {
    /* Reset the timer after reaching match register value */
    reg->MCR |= mask;
  }
  else
    reg->MCR &= ~mask;

  reg->TCR = state;
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
  const uint32_t state = reg->TCR & TCR_CEN;

  assert(value <= reg->MR[timer->event]);

  reg->TCR = 0;

  reg->PC = 0;
  reg->TC = value;

  reg->TCR = state;
}
