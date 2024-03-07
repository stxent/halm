/*
 * rit.c
 * Copyright (C) 2018 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/platform/lpc/rit.h>
#include <halm/platform/lpc/rit_defs.h>
#include <halm/platform/lpc/rit_base.h>
#include <halm/platform/platform_defs.h>
/*----------------------------------------------------------------------------*/
static bool setInstance(struct Rit *);
/*----------------------------------------------------------------------------*/
static enum Result tmrInit(void *, const void *);
static void tmrEnable(void *);
static void tmrDisable(void *);
static void tmrSetCallback(void *, void (*)(void *), void *);
static uint32_t tmrGetFrequency(const void *);
static uint32_t tmrGetOverflow(const void *);
static void tmrSetOverflow(void *, uint32_t);
static uint32_t tmrGetValue(const void *);
static void tmrSetValue(void *, uint32_t);

#ifndef CONFIG_PLATFORM_LPC_RIT_NO_DEINIT
static void tmrDeinit(void *);
#else
#  define tmrDeinit deletedDestructorTrap
#endif
/*----------------------------------------------------------------------------*/
const struct TimerClass * const Rit = &(const struct TimerClass){
    .size = sizeof(struct Rit),
    .init = tmrInit,
    .deinit = tmrDeinit,

    .enable = tmrEnable,
    .disable = tmrDisable,
    .setAutostop = NULL,
    .setCallback = tmrSetCallback,
    .getFrequency = tmrGetFrequency,
    .setFrequency = NULL,
    .getOverflow = tmrGetOverflow,
    .setOverflow = tmrSetOverflow,
    .getValue = tmrGetValue,
    .setValue = tmrSetValue
};
/*----------------------------------------------------------------------------*/
static struct Rit *instance = NULL;
/*----------------------------------------------------------------------------*/
static bool setInstance(struct Rit *object)
{
  if (instance == NULL)
  {
    instance = object;
    return true;
  }
  else
    return false;
}
/*----------------------------------------------------------------------------*/
void RIT_ISR(void)
{
  const uint32_t control = LPC_RIT->CTRL;

  if (control & CTRL_RITINT)
  {
    /* Clear pending interrupt flag */
    LPC_RIT->CTRL = control;

    if (instance->callback != NULL)
      instance->callback(instance->callbackArgument);
  }
}
/*----------------------------------------------------------------------------*/
static enum Result tmrInit(void *object, const void *configBase)
{
  const struct RitConfig * const config = configBase;
  struct Rit * const timer = object;

  if (setInstance(timer))
  {
    ritBaseInit();

    /* Configure the timer but leave it in the disabled state */
    LPC_RIT->CTRL = CTRL_RITENCLR | CTRL_RITENBR;
    LPC_RIT->COMPVAL = TIMER_RESOLUTION;

    if (config != NULL)
      irqSetPriority(RIT_IRQ, config->priority);
    irqEnable(RIT_IRQ);

    return E_OK;
  }
  else
    return E_BUSY;
}
/*----------------------------------------------------------------------------*/
#ifndef CONFIG_PLATFORM_LPC_RIT_NO_DEINIT
static void tmrDeinit(void *)
{
  irqDisable(RIT_IRQ);
  LPC_RIT->CTRL = 0;
  ritBaseDeinit();

  instance = NULL;
}
#endif
/*----------------------------------------------------------------------------*/
static void tmrEnable(void *)
{
  LPC_RIT->CTRL |= CTRL_RITEN;
}
/*----------------------------------------------------------------------------*/
static void tmrDisable(void *)
{
  LPC_RIT->CTRL &= ~CTRL_RITEN;
}
/*----------------------------------------------------------------------------*/
static void tmrSetCallback(void *object, void (*callback)(void *),
    void *argument)
{
  struct Rit * const timer = object;

  timer->callback = callback;
  timer->callbackArgument = argument;
}
/*----------------------------------------------------------------------------*/
static uint32_t tmrGetFrequency(const void *)
{
  return ritGetClock();
}
/*----------------------------------------------------------------------------*/
static uint32_t tmrGetOverflow(const void *)
{
  return LPC_RIT->COMPVAL + 1;
}
/*----------------------------------------------------------------------------*/
static void tmrSetOverflow(void *, uint32_t overflow)
{
  LPC_RIT->COMPVAL = overflow - 1;
}
/*----------------------------------------------------------------------------*/
static uint32_t tmrGetValue(const void *)
{
  return LPC_RIT->COUNTER;
}
/*----------------------------------------------------------------------------*/
static void tmrSetValue(void *, uint32_t value)
{
  LPC_RIT->COUNTER = value;
}
