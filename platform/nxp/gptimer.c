/*
 * gptimer.c
 * Copyright (C) 2013 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <assert.h>
#include <platform/nxp/gptimer.h>
#include <platform/nxp/gptimer_defs.h>
#include <pm.h>
/*----------------------------------------------------------------------------*/
static inline uint32_t getResolution(struct GpTimer *);
static void interruptHandler(void *);
static enum result updateFrequency(struct GpTimer *, uint32_t);
/*----------------------------------------------------------------------------*/
#ifdef CONFIG_GPTIMER_PM
static enum result powerStateHandler(void *, enum pmState);
#endif
/*----------------------------------------------------------------------------*/
static enum result tmrInit(void *, const void *);
static void tmrDeinit(void *);
static void tmrCallback(void *, void (*)(void *), void *);
static void tmrSetEnabled(void *, bool);
static enum result tmrSetFrequency(void *, uint32_t);
static enum result tmrSetOverflow(void *, uint32_t);
static enum result tmrSetValue(void *, uint32_t);
static uint32_t tmrValue(const void *);
/*----------------------------------------------------------------------------*/
static const struct TimerClass tmrTable = {
    .size = sizeof(struct GpTimer),
    .init = tmrInit,
    .deinit = tmrDeinit,

    .callback = tmrCallback,
    .setEnabled = tmrSetEnabled,
    .setFrequency = tmrSetFrequency,
    .setOverflow = tmrSetOverflow,
    .setValue = tmrSetValue,
    .value = tmrValue
};
/*----------------------------------------------------------------------------*/
const struct TimerClass * const GpTimer = &tmrTable;
/*----------------------------------------------------------------------------*/
static inline uint32_t getResolution(struct GpTimer *timer)
{
  return (1UL << timer->base.resolution) - 1;
}
/*----------------------------------------------------------------------------*/
static void interruptHandler(void *object)
{
  struct GpTimer * const timer = object;
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
#ifdef CONFIG_GPTIMER_PM
static enum result powerStateHandler(void *object, enum pmState state)
{
  struct GpTimer * const timer = object;

  return state == PM_ACTIVE ? updateFrequency(timer, timer->frequency) : E_OK;
}
#endif
/*----------------------------------------------------------------------------*/
static enum result updateFrequency(struct GpTimer *timer, uint32_t frequency)
{
  LPC_TIMER_Type * const reg = timer->base.reg;

  if (frequency)
  {
    const uint32_t baseClock = gpTimerGetClock((struct GpTimerBase *)timer);
    const uint32_t divisor = baseClock / frequency - 1;

    if (frequency > baseClock || divisor > getResolution(timer))
      return E_VALUE;

    reg->PR = divisor;
  }
  else
    reg->PR = 0;

  timer->frequency = frequency;
  return E_OK;
}
/*----------------------------------------------------------------------------*/
static enum result tmrInit(void *object, const void *configBase)
{
  const struct GpTimerConfig * const config = configBase;
  const struct GpTimerBaseConfig baseConfig = {
      .channel = config->channel
  };
  struct GpTimer * const timer = object;
  enum result res;

  assert(config->event < GPTIMER_EVENT_END);

  if (!config->event)
  {
    /* Only 1 channel is used, allocation always succeeds */
    timer->event = gpTimerAllocateChannel(0);
  }
  else
    timer->event = config->event - 1;

  /* Call base class constructor */
  if ((res = GpTimerBase->init(object, &baseConfig)) != E_OK)
    return res;

  timer->base.handler = interruptHandler;
  timer->callback = 0;

  /* Initialize peripheral block */
  LPC_TIMER_Type * const reg = timer->base.reg;

  reg->TCR = TCR_CRES;

  reg->IR = reg->IR; /* Clear pending interrupts */
  reg->CCR = 0;
  reg->CTCR = 0;
  reg->EMR = 0;
  reg->MCR = 0;

  if ((res = updateFrequency(timer, config->frequency)) != E_OK)
    return res;

  /* Configure prescaler and default match value */
  reg->MR[timer->event] = getResolution(timer);

#ifdef CONFIG_GPTIMER_PM
  if ((res = pmRegister(interface, powerStateHandler)) != E_OK)
    return res;
#endif

  /* Timer is disabled by default */
  reg->TCR = 0;

  irqSetPriority(timer->base.irq, config->priority);
  irqEnable(timer->base.irq);

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static void tmrDeinit(void *object)
{
  struct GpTimer * const timer = object;
  LPC_TIMER_Type * const reg = timer->base.reg;

  irqDisable(timer->base.irq);
  reg->TCR = 0;

#ifdef CONFIG_GPTIMER_PM
  pmUnregister(interface);
#endif

  GpTimerBase->deinit(timer);
}
/*----------------------------------------------------------------------------*/
static void tmrCallback(void *object, void (*callback)(void *), void *argument)
{
  struct GpTimer * const timer = object;
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
  struct GpTimer * const timer = object;
  LPC_TIMER_Type * const reg = timer->base.reg;

  /* Temporarily disable timer */
  reg->TCR = 0;
  /* Clear pending interrupt flags and direct memory access requests */
  reg->IR = IR_MATCH_INTERRUPT(timer->event);

  if (state)
  {
    /* Clear match value to avoid undefined output level */
    reg->EMR &= ~EMR_EXTERNAL_MATCH(timer->event);
    reg->TCR = TCR_CEN;
  }
}
/*----------------------------------------------------------------------------*/
static enum result tmrSetFrequency(void *object, uint32_t frequency)
{
  struct GpTimer * const timer = object;

  return updateFrequency(timer, frequency);
}
/*----------------------------------------------------------------------------*/
static enum result tmrSetOverflow(void *object, uint32_t overflow)
{
  struct GpTimer * const timer = object;
  LPC_TIMER_Type * const reg = timer->base.reg;
  const uint32_t resolution = getResolution(timer);
  const bool enabled = reg->TCR & TCR_CEN;

  if (overflow)
  {
    overflow = overflow - 1;

    if (overflow > resolution)
      return E_VALUE;

    /* Reset and stop timer before configuration or simply stop it */
    if (reg->MR[timer->event] >= overflow)
      reg->TCR = TCR_CRES;
    else
      reg->TCR = 0;

    reg->MR[timer->event] = overflow;
    /* Enable timer reset after reaching match register value */
    reg->MCR |= MCR_RESET(timer->event);
    /* Enable external match to generate signals to other peripherals */
    reg->EMR |= EMR_CONTROL(timer->event, CONTROL_TOGGLE);
  }
  else
  {
    reg->TCR = 0;

    reg->MR[timer->event] = resolution;
    /* Disable timer reset and clear external match output value */
    reg->MCR &= ~MCR_RESET(timer->event);
    reg->EMR &= ~EMR_CONTROL_MASK(timer->event);
  }

  reg->TCR = enabled ? TCR_CEN : 0;

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static enum result tmrSetValue(void *object, uint32_t value)
{
  struct GpTimer * const timer = object;
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
/*----------------------------------------------------------------------------*/
static uint32_t tmrValue(const void *object)
{
  const struct GpTimer * const timer = object;
  const LPC_TIMER_Type * const reg = timer->base.reg;

  return reg->TC;
}
