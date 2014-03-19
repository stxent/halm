/*
 * gptimer.c
 * Copyright (C) 2013 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <assert.h>
#include <platform/nxp/gptimer.h>
#include <platform/nxp/gptimer_defs.h>
/*----------------------------------------------------------------------------*/
#define DEFAULT_OVERFLOW 0xFFFFFFFFUL
/*----------------------------------------------------------------------------*/
static void interruptHandler(void *);
/*----------------------------------------------------------------------------*/
static enum result tmrInit(void *, const void *);
static void tmrDeinit(void *);
static void tmrCallback(void *, void (*)(void *), void *);
static void tmrSetEnabled(void *, bool);
static void tmrSetFrequency(void *, uint32_t);
static void tmrSetOverflow(void *, uint32_t);
static uint32_t tmrValue(void *);
/*----------------------------------------------------------------------------*/
static const struct TimerClass timerTable = {
    .size = sizeof(struct GpTimer),
    .init = tmrInit,
    .deinit = tmrDeinit,

    .callback = tmrCallback,
    .setEnabled = tmrSetEnabled,
    .setFrequency = tmrSetFrequency,
    .setOverflow = tmrSetOverflow,
    .value = tmrValue
};
/*----------------------------------------------------------------------------*/
const struct TimerClass *GpTimer = &timerTable;
/*----------------------------------------------------------------------------*/
static void interruptHandler(void *object)
{
  struct GpTimer *timer = object;
  LPC_TIMER_Type *reg = timer->parent.reg;

  reg->IR = reg->IR; /* Clear all pending interrupts */

  if (timer->callback)
    timer->callback(timer->callbackArgument);
}
/*----------------------------------------------------------------------------*/
static enum result tmrInit(void *object, const void *configPtr)
{
  const struct GpTimerConfig * const config = configPtr;
  const struct GpTimerBaseConfig parentConfig = {
      .channel = config->channel,
      .input = config->input
  };
  struct GpTimer *timer = object;
  enum result res;

  assert(config->frequency && config->event < 4);

  /* Call base class constructor */
  if ((res = GpTimerBase->init(object, &parentConfig)) != E_OK)
    return res;

  timer->parent.handler = interruptHandler;
  /* Match channel used as event source for timer reset */
  timer->event = config->event;

  LPC_TIMER_Type *reg = timer->parent.reg;

  reg->TCR = 0;

  reg->IR = reg->IR; /* Clear pending interrupts */
  reg->PC = reg->TC = 0;
  reg->CTCR = 0;
  reg->CCR = 0;
  reg->EMR = 0;

  /* Configure prescaler and default match value */
  reg->PR = gpTimerGetClock(object) / config->frequency - 1;
  reg->MR[timer->event] = DEFAULT_OVERFLOW;
  reg->MCR = 0; /* All match channels are currently disabled */

  /* Enable counter */
  reg->TCR = TCR_CEN;

  irqSetPriority(timer->parent.irq, config->priority);
  irqEnable(timer->parent.irq);

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static void tmrDeinit(void *object)
{
  struct GpTimer *timer = object;

  irqDisable(timer->parent.irq);
  ((LPC_TIMER_Type *)timer->parent.reg)->TCR &= ~TCR_CEN;
  GpTimerBase->deinit(timer);
}
/*----------------------------------------------------------------------------*/
static void tmrCallback(void *object, void (*callback)(void *), void *argument)
{
  struct GpTimer *timer = object;
  LPC_TIMER_Type *reg = timer->parent.reg;

  timer->callbackArgument = argument;
  timer->callback = callback;

  reg->IR = reg->IR; /* Clear all pending interrupts */
  if (callback)
    reg->MCR |= MCR_INTERRUPT(timer->event);
  else
    reg->MCR &= ~MCR_INTERRUPT(timer->event);
}
/*----------------------------------------------------------------------------*/
static void tmrSetEnabled(void *object, bool state)
{
  struct GpTimer *timer = object;
  LPC_TIMER_Type *reg = timer->parent.reg;

  if (!state)
  {
    reg->TCR |= TCR_CRES;
    while (reg->TC || reg->PC);
  }
  else
  {
    /* Clear match value to avoid undefined output level */
    reg->EMR &= ~EMR_EXTERNAL_MATCH(timer->event);
    reg->TCR &= ~TCR_CRES;
  }
}
/*----------------------------------------------------------------------------*/
static void tmrSetFrequency(void *object, uint32_t frequency)
{
  assert(frequency);

  ((LPC_TIMER_Type *)((struct GpTimer *)object)->parent.reg)->PR =
      gpTimerGetClock(object) / frequency - 1;
}
/*----------------------------------------------------------------------------*/
static void tmrSetOverflow(void *object, uint32_t overflow)
{
  struct GpTimer *timer = object;
  LPC_TIMER_Type *reg = timer->parent.reg;

  if (overflow)
  {
    reg->MR[timer->event] = overflow - 1;
    /* Enable timer reset after reaching match register value */
    reg->MCR |= MCR_RESET(timer->event);
    /* Enable external match to generate signals to other peripherals */
    reg->EMR |= EMR_CONTROL(timer->event, EMR_CONTROL_TOGGLE);
  }
  else
  {
    reg->MR[timer->event] = DEFAULT_OVERFLOW;
    /* Disable timer reset and clear external match output value */
    reg->MCR &= ~MCR_RESET(timer->event);
    reg->EMR &= ~EMR_CONTROL_MASK(timer->event);
  }

  if (reg->TCR & TCR_CRES)
  {
    /* Synchronously reset prescaler and counter registers */
    reg->TCR |= TCR_CRES;
    while (reg->TC || reg->PC);
    reg->TCR &= ~TCR_CRES;
  }
}
/*----------------------------------------------------------------------------*/
static uint32_t tmrValue(void *object)
{
  return ((LPC_TIMER_Type *)((struct GpTimer *)object)->parent.reg)->TC;
}
