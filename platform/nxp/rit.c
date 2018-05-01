/*
 * rit.c
 * Copyright (C) 2018 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <halm/platform/nxp/rit.h>
#include <halm/platform/nxp/rit_defs.h>
#include <halm/platform/nxp/rit_base.h>
#include <halm/platform/platform_defs.h>
/*----------------------------------------------------------------------------*/
static void resetInstance(void);
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

#ifndef CONFIG_PLATFORM_NXP_RIT_NO_DEINIT
static void tmrDeinit(void *);
#else
#define tmrDeinit deletedDestructorTrap
#endif
/*----------------------------------------------------------------------------*/
static const struct TimerClass tmrTable = {
    .size = sizeof(struct Rit),
    .init = tmrInit,
    .deinit = tmrDeinit,

    .enable = tmrEnable,
    .disable = tmrDisable,
    .setCallback = tmrSetCallback,
    .getFrequency = tmrGetFrequency,
    .setFrequency = 0,
    .getOverflow = tmrGetOverflow,
    .setOverflow = tmrSetOverflow,
    .getValue = tmrGetValue,
    .setValue = tmrSetValue
};
/*----------------------------------------------------------------------------*/
const struct TimerClass * const Rit = &tmrTable;
static struct Rit *instance = 0;
/*----------------------------------------------------------------------------*/
static void resetInstance(void)
{
  instance = 0;
}
/*----------------------------------------------------------------------------*/
static bool setInstance(struct Rit *object)
{
  if (!instance)
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
  /* Clear pending interrupt flag */
  LPC_RIT->CTRL = LPC_RIT->CTRL;

  instance->callback(instance->callbackArgument);
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

    if (config)
      irqSetPriority(RIT_IRQ, config->priority);

    return E_OK;
  }
  else
    return E_BUSY;
}
/*----------------------------------------------------------------------------*/
#ifndef CONFIG_PLATFORM_NXP_RIT_NO_DEINIT
static void tmrDeinit(void *object __attribute__((unused)))
{
  irqDisable(RIT_IRQ);
  LPC_RIT->CTRL = 0;
  ritBaseDeinit();
  resetInstance();
}
#endif
/*----------------------------------------------------------------------------*/
static void tmrEnable(void *object __attribute__((unused)))
{
  LPC_RIT->CTRL |= CTRL_RITEN;
}
/*----------------------------------------------------------------------------*/
static void tmrDisable(void *object __attribute__((unused)))
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

  if (timer->callback)
    irqEnable(RIT_IRQ);
  else
    irqDisable(RIT_IRQ);
}
/*----------------------------------------------------------------------------*/
static uint32_t tmrGetFrequency(const void *object __attribute__((unused)))
{
  return ritGetClock();
}
/*----------------------------------------------------------------------------*/
static uint32_t tmrGetOverflow(const void *object __attribute__((unused)))
{
  return LPC_RIT->COMPVAL + 1;
}
/*----------------------------------------------------------------------------*/
static void tmrSetOverflow(void *object __attribute__((unused)),
    uint32_t overflow)
{
  LPC_RIT->COMPVAL = overflow - 1;
}
/*----------------------------------------------------------------------------*/
static uint32_t tmrGetValue(const void *object __attribute__((unused)))
{
  return LPC_RIT->COUNTER;
}
/*----------------------------------------------------------------------------*/
static void tmrSetValue(void *object __attribute__((unused)), uint32_t value)
{
  LPC_RIT->COUNTER = value;
}
