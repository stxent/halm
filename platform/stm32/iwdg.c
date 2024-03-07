/*
 * iwdg.c
 * Copyright (C) 2023 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/platform/stm32/iwdg.h>
#include <halm/platform/stm32/iwdg_defs.h>
#include <halm/platform/platform_defs.h>
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
    .setCallback = NULL,
    .reload = wdtReload
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

  const uint32_t frequency = iwdgGetClock(object);
  if (!frequency)
    return E_ERROR;

  const uint32_t prescaler = config->period * frequency / 1000;
  uint32_t divider = 2;
  uint32_t reload;

  while (divider < 8 && (prescaler >> divider) > RLR_RL_MAX)
    ++divider;

  reload = config->period * (frequency / (1UL << divider)) / 1000;
  if (reload > RLR_RL_MAX)
    reload = RLR_RL_MAX;

  STM_IWDG->KR = KR_UNLOCK;
  STM_IWDG->RLR = reload;
  STM_IWDG->KR = KR_UNLOCK;
  STM_IWDG->PR = divider;

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
static void wdtReload([[maybe_unused]] void *object)
{
  STM_IWDG->KR = KR_RELOAD;
}
