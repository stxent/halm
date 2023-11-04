/*
 * gptimer.c
 * Copyright (C) 2023 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/platform/numicro/gptimer.h>
#include <halm/platform/numicro/gptimer_defs.h>
#include <halm/platform/platform_defs.h>
#include <halm/pm.h>
#include <assert.h>
/*----------------------------------------------------------------------------*/
static void interruptHandler(void *);

#ifdef CONFIG_PLATFORM_NUMICRO_GPTIMER_PM
static void powerStateHandler(void *, enum PmState);
#endif
/*----------------------------------------------------------------------------*/
static enum Result tmrInit(void *, const void *);
static void tmrEnable(void *);
static void tmrDisable(void *);
static void tmrSetAutostop(void *, bool);
static void tmrSetCallback(void *, void (*)(void *), void *);
static uint32_t tmrGetFrequency(const void *);
static void tmrSetFrequency(void *, uint32_t);
static uint32_t tmrGetOverflow(const void *);
static void tmrSetOverflow(void *, uint32_t);
static uint32_t tmrGetValue(const void *);
static void tmrSetValue(void *, uint32_t);

#ifndef CONFIG_PLATFORM_NUMICRO_GPTIMER_NO_DEINIT
static void tmrDeinit(void *);
#else
#define tmrDeinit deletedDestructorTrap
#endif
/*----------------------------------------------------------------------------*/
const struct TimerClass * const GpTimer = &(const struct TimerClass){
    .size = sizeof(struct GpTimer),
    .init = tmrInit,
    .deinit = tmrDeinit,

    .enable = tmrEnable,
    .disable = tmrDisable,
    .setAutostop = tmrSetAutostop,
    .setCallback = tmrSetCallback,
    .getFrequency = tmrGetFrequency,
    .setFrequency = tmrSetFrequency,
    .getOverflow = tmrGetOverflow,
    .setOverflow = tmrSetOverflow,
    .getValue = tmrGetValue,
    .setValue = tmrSetValue
};
/*----------------------------------------------------------------------------*/
static void interruptHandler(void *object)
{
  struct GpTimer * const timer = object;
  NM_TIMER_Type * const reg = timer->base.reg;

  /* Clear all pending interrupts */
  reg->INTSTS = INTSTS_TIF | INTSTS_TWKF;

  timer->callback(timer->callbackArgument);
}
/*----------------------------------------------------------------------------*/
#ifdef CONFIG_PLATFORM_NUMICRO_GPTIMER_PM
static void powerStateHandler(void *object, enum PmState state)
{
  if (state == PM_ACTIVE)
  {
    struct GpTimer * const timer = object;
    gpTimerSetTimerFrequency(&timer->base, timer->frequency);
  }
}
#endif
/*----------------------------------------------------------------------------*/
static enum Result tmrInit(void *object, const void *configBase)
{
  const struct GpTimerConfig * const config = configBase;
  assert(config != NULL);

  const struct GpTimerBaseConfig baseConfig = {
      .channel = config->channel
  };
  struct GpTimer * const timer = object;
  enum Result res;

  /* Call base class constructor */
  if ((res = GpTimerBase->init(timer, &baseConfig)) != E_OK)
    return res;

  timer->base.handler = interruptHandler;
  timer->callback = NULL;

  /* Initialize peripheral block */
  NM_TIMER_Type * const reg = timer->base.reg;
  uint32_t ctl = CTL_OPMODE(OPMODE_PERIODIC);

  if (config->trigger.adc)
    ctl |= CTL_TRGADC;
  if (config->trigger.dma)
    ctl |= CTL_TRGPDMA;
  if (config->trigger.pwm)
    ctl |= CTL_TRGPWM | CTL_TRGBPWM;
  if (config->trigger.wakeup)
    ctl |= CTL_WKEN;

  reg->CTL = CTL_RSTCNT;
  while (reg->CTL & CTL_ACTSTS);

  reg->CTL = ctl;
  reg->CMP = CMP_CMPDAT_MASK;
  reg->EXTCTL = 0;
  reg->INTSTS = INTSTS_TIF | INTSTS_TWKF;
  reg->EINTSTS = EINTSTS_CAPIF;

  timer->frequency = config->frequency;
  gpTimerSetTimerFrequency(&timer->base, timer->frequency);

#ifdef CONFIG_PLATFORM_NUMICRO_GPTIMER_PM
  if ((res = pmRegister(powerStateHandler, timer)) != E_OK)
    return res;
#endif

  irqSetPriority(timer->base.irq, config->priority);
  irqEnable(timer->base.irq);

  return E_OK;
}
/*----------------------------------------------------------------------------*/
#ifndef CONFIG_PLATFORM_NUMICRO_GPTIMER_NO_DEINIT
static void tmrDeinit(void *object)
{
  struct GpTimer * const timer = object;
  NM_TIMER_Type * const reg = timer->base.reg;

  reg->CTL = 0;
  irqDisable(timer->base.irq);

#ifdef CONFIG_PLATFORM_NUMICRO_GPTIMER_PM
  pmUnregister(timer);
#endif

  GpTimerBase->deinit(timer);
}
#endif
/*----------------------------------------------------------------------------*/
static void tmrEnable(void *object)
{
  struct GpTimer * const timer = object;
  NM_TIMER_Type * const reg = timer->base.reg;

  /* Clear pending interrupt flags */
  reg->INTSTS = INTSTS_TIF | INTSTS_TWKF;
  /* Start the timer, operation needs 2 * TMR_CLK period to complete */
  reg->CTL |= CTL_CNTEN;
}
/*----------------------------------------------------------------------------*/
static void tmrDisable(void *object)
{
  struct GpTimer * const timer = object;
  NM_TIMER_Type * const reg = timer->base.reg;

  /* Stop the timer, operation needs 2 * TMR_CLK period to complete */
  reg->CTL &= ~CTL_CNTEN;
}
/*----------------------------------------------------------------------------*/
static void tmrSetAutostop(void *object, bool state)
{
  struct GpTimer * const timer = object;
  NM_TIMER_Type * const reg = timer->base.reg;
  uint32_t ctl = reg->CTL & ~CTL_OPMODE_MASK;

  if (state)
    ctl |= CTL_OPMODE(OPMODE_ONE_SHOT);
  else
    ctl |= CTL_OPMODE(OPMODE_PERIODIC);

  reg->CTL = ctl;
}
/*----------------------------------------------------------------------------*/
static void tmrSetCallback(void *object, void (*callback)(void *),
    void *argument)
{
  struct GpTimer * const timer = object;
  NM_TIMER_Type * const reg = timer->base.reg;

  timer->callbackArgument = argument;
  timer->callback = callback;

  if (timer->callback != NULL)
  {
    reg->INTSTS = INTSTS_TIF | INTSTS_TWKF;
    reg->CTL |= CTL_INTEN;
  }
  else
    reg->CTL &= ~CTL_INTEN;
}
/*----------------------------------------------------------------------------*/
static uint32_t tmrGetFrequency(const void *object)
{
  const struct GpTimer * const timer = object;
  const NM_TIMER_Type * const reg = timer->base.reg;
  const uint32_t apbClock = gpTimerGetClock(&timer->base);

  return apbClock / (CTL_PSC_VALUE(reg->CTL) + 1);
}
/*----------------------------------------------------------------------------*/
static void tmrSetFrequency(void *object, uint32_t frequency)
{
  struct GpTimer * const timer = object;

  timer->frequency = frequency;
  gpTimerSetTimerFrequency(&timer->base, timer->frequency);
}
/*----------------------------------------------------------------------------*/
static uint32_t tmrGetOverflow(const void *object)
{
  const struct GpTimer * const timer = object;
  const NM_TIMER_Type * const reg = timer->base.reg;

  return reg->CMP;
}
/*----------------------------------------------------------------------------*/
static void tmrSetOverflow(void *object, uint32_t overflow)
{
  struct GpTimer * const timer = object;
  NM_TIMER_Type * const reg = timer->base.reg;

  assert(overflow > 1 && overflow <= CMP_CMPDAT_MASK);
  reg->CMP = overflow;
}
/*----------------------------------------------------------------------------*/
static uint32_t tmrGetValue(const void *object)
{
  const struct GpTimer * const timer = object;
  const NM_TIMER_Type * const reg = timer->base.reg;

  return reg->CNT;
}
/*----------------------------------------------------------------------------*/
static void tmrSetValue(void *object, uint32_t value)
{
  struct GpTimer * const timer = object;
  NM_TIMER_Type * const reg = timer->base.reg;
  const bool enabled = (reg->CTL & CTL_CNTEN) != 0;

  /* Data register is read-only, writing 0 is implemented using reset */
  assert(value == 0);
  (void)value;

  reg->CTL |= CTL_RSTCNT;

  if (enabled)
  {
    while (reg->CTL & CTL_ACTSTS);
    reg->CTL |= CTL_CNTEN;
  }
}
