/*
 * iwdg.c
 * Copyright (C) 2023 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/platform/stm32/iwdg.h>
#include <halm/platform/stm32/iwdg_defs.h>
#include <halm/platform/platform_defs.h>
#include <xcore/accel.h>
#include <assert.h>
/*----------------------------------------------------------------------------*/
static enum Result wdtInit(void *, const void *);
static bool wdtFired(const void *);
static void wdtReload(void *);
/*----------------------------------------------------------------------------*/
const struct WatchdogClass * const Iwdg = &(const struct WatchdogClass){
    .size = sizeof(struct Iwdg),
    .init = wdtInit,
    .deinit = NULL, /* Default destructor */

    .fired = wdtFired,
    .reload = wdtReload,
    .setCallback = NULL
};
/*----------------------------------------------------------------------------*/
static enum Result wdtInit(void *object, const void *configBase)
{
  const struct IwdgConfig * const config = configBase;
  assert(config != NULL);

  struct Iwdg * const timer = object;

  /* Call base class constructor */
  const enum Result res = IwdgBase->init(timer, NULL);
  if (res != E_OK)
    return res;

  const uint32_t frequency = iwdgGetClock(object) / 1000;
  if (!frequency)
    return E_ERROR;

  const uint64_t period = (uint64_t)frequency * config->period;
  if (period > RLR_RL_MAX_DIV)
    return E_VALUE;

  const uint32_t resolution = 31 - countLeadingZeros32((uint32_t)period);
  const uint32_t prescaler = resolution >= RLR_RL_MAX_POW ?
      resolution - (RLR_RL_MAX_POW - 1) : 2;

  STM_IWDG->KR = KR_UNLOCK;
  STM_IWDG->RLR = (uint32_t)period >> prescaler;
  STM_IWDG->KR = KR_UNLOCK;
  STM_IWDG->PR = prescaler - 2;

  /* Enable counter */
  STM_IWDG->KR = KR_START;
  STM_IWDG->KR = KR_RELOAD;

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static bool wdtFired(const void *object)
{
  const struct Iwdg * const timer = object;
  return timer->base.fired;
}
/*----------------------------------------------------------------------------*/
static void wdtReload(void *)
{
  STM_IWDG->KR = KR_RELOAD;
}
