/*
 * atimer.c
 * Copyright (C) 2023 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/platform/lpc/clocking.h>
#include <halm/platform/lpc/lpc43xx/atimer.h>
#include <halm/platform/lpc/lpc43xx/atimer_defs.h>
#include <halm/platform/lpc/lpc43xx/event_router.h>
#include <halm/platform/platform_defs.h>
#include <assert.h>
/*----------------------------------------------------------------------------*/
#define ATIMER_DIVIDER 32
/*----------------------------------------------------------------------------*/
static void interruptHandler(void *);
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

#ifndef CONFIG_PLATFORM_LPC_ATIMER_NO_DEINIT
static void tmrDeinit(void *);
#else
#  define tmrDeinit deletedDestructorTrap
#endif
/*----------------------------------------------------------------------------*/
const struct TimerClass * const Atimer = &(const struct TimerClass){
    .size = sizeof(struct Atimer),
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
static struct Atimer *instance = NULL;
/*----------------------------------------------------------------------------*/
static void interruptHandler(void *object)
{
  struct Atimer * const timer = object;

  /* Clear pending interrupt flag */
  LPC_ATIMER->CLR_STAT = CLR_STAT_CSTAT;

  if (timer->callback != NULL)
    timer->callback(timer->callbackArgument);
}
/*----------------------------------------------------------------------------*/
static bool setInstance(struct Atimer *object)
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
static enum Result tmrInit(void *object,
    [[maybe_unused]] const void *configBase)
{
  struct Atimer * const timer = object;

  if (!setInstance(timer))
    return E_BUSY;

  timer->callback = NULL;

  LPC_ATIMER->CLR_EN = CLR_EN_CLR_EN;
  LPC_ATIMER->CLR_STAT = CLR_STAT_CSTAT;

  LPC_ATIMER->PRESET = PRESET_MAX;
  while (LPC_ATIMER->PRESET != PRESET_MAX);
  LPC_ATIMER->DOWNCOUNTER = PRESET_MAX;

  return erRegister(interruptHandler, timer, ER_ATIMER);
}
/*----------------------------------------------------------------------------*/
#ifndef CONFIG_PLATFORM_LPC_ATIMER_NO_DEINIT
static void tmrDeinit(void *object)
{
  struct Atimer * const timer = object;

  LPC_ATIMER->CLR_EN = CLR_EN_CLR_EN;

  erUnregister(timer);
  instance = NULL;
}
#endif
/*----------------------------------------------------------------------------*/
static void tmrEnable([[maybe_unused]] void *object)
{
  /* Timer is free-running, reload current value and enable interrupts */
  LPC_ATIMER->DOWNCOUNTER = LPC_ATIMER->PRESET;

  LPC_ATIMER->CLR_STAT = CLR_STAT_CSTAT;
  LPC_ATIMER->SET_EN = SET_EN_SET_EN;
}
/*----------------------------------------------------------------------------*/
static void tmrDisable([[maybe_unused]] void *object)
{
  LPC_ATIMER->CLR_EN = CLR_EN_CLR_EN;
}
/*----------------------------------------------------------------------------*/
static void tmrSetCallback(void *object, void (*callback)(void *),
    void *argument)
{
  struct Atimer * const timer = object;

  timer->callbackArgument = argument;
  timer->callback = callback;
}
/*----------------------------------------------------------------------------*/
static uint32_t tmrGetFrequency([[maybe_unused]] const void *object)
{
  return clockFrequency(RtcOsc) / ATIMER_DIVIDER;
}
/*----------------------------------------------------------------------------*/
static uint32_t tmrGetOverflow([[maybe_unused]] const void *object)
{
  return LPC_ATIMER->PRESET + 1;
}
/*----------------------------------------------------------------------------*/
static void tmrSetOverflow([[maybe_unused]] void *object, uint32_t overflow)
{
  const uint32_t value = overflow ? overflow - 1 : PRESET_MAX;

  assert(value <= PRESET_MAX);

  LPC_ATIMER->PRESET = value;
  while (LPC_ATIMER->PRESET != value);
}
/*----------------------------------------------------------------------------*/
static uint32_t tmrGetValue([[maybe_unused]] const void *object)
{
  return LPC_ATIMER->DOWNCOUNTER;
}
/*----------------------------------------------------------------------------*/
static void tmrSetValue([[maybe_unused]] void *object, uint32_t value)
{
  assert(value <= PRESET_MAX);
  LPC_ATIMER->DOWNCOUNTER = value;
}
