/*
 * gptimer.c
 * Copyright (C) 2013 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <assert.h>
#include <halm/platform/nxp/gptimer.h>
#include <halm/platform/nxp/gptimer_defs.h>
#include <halm/pm.h>
/*----------------------------------------------------------------------------*/
static inline uint32_t getMaxValue(struct GpTimer *);
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
static inline uint32_t getMaxValue(struct GpTimer *timer)
{
  return MASK(timer->base.resolution);
}
/*----------------------------------------------------------------------------*/
static void interruptHandler(void *object)
{
  struct GpTimer * const timer = object;
  LPC_TIMER_Type * const reg = timer->base.reg;

  /* Clear all pending interrupts */
  reg->IR = reg->IR;

  if (timer->callback)
    timer->callback(timer->callbackArgument);
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

    if (frequency > baseClock || divisor > getMaxValue(timer))
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

  timer->event = config->event ? config->event : GPTIMER_MATCH0;
  --timer->event;

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
  reg->MCR = 0;

  if ((res = updateFrequency(timer, config->frequency)) != E_OK)
    return res;

  /* Configure prescaler and default match value */
  reg->MR[timer->event] = getMaxValue(timer);
  /* Enable external match to generate signals to other peripherals */
  reg->EMR = EMR_CONTROL(timer->event, CONTROL_TOGGLE);

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

  /* Stop the timer */
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
  const uint32_t resolution = getMaxValue(timer);
  const bool enabled = reg->TCR & TCR_CEN;

  /* Stop the timer before reconfiguration */
  reg->TCR = 0;

  assert((overflow - 1) <= resolution);
  overflow = (overflow - 1) & resolution;

  /* Setup the match register */
  reg->MR[timer->event] = overflow;

  if (overflow != resolution)
  {
    /* Reset the timer after reaching match register value */
    reg->MCR |= MCR_RESET(timer->event);
  }
  else
    reg->MCR &= ~MCR_RESET(timer->event);

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
