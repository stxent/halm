/*
 * iwdg_base.c
 * Copyright (C) 2023 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/platform/stm32/clocking.h>
#include <halm/platform/stm32/iwdg_base.h>
#include <halm/platform/stm32/iwdg_defs.h>
#include <halm/platform/stm32/stm32f1xx/clocking_defs.h>
/*----------------------------------------------------------------------------*/
static bool setInstance(struct IwdgBase *);

static enum Result wdtInit(void *, const void *);
/*----------------------------------------------------------------------------*/
const struct EntityClass * const IwdgBase = &(const struct EntityClass){
    .size = 0, /* Abstract class */
    .init = wdtInit,
    .deinit = NULL /* Default destructor */
};
/*----------------------------------------------------------------------------*/
static struct IwdgBase *instance = NULL;
/*----------------------------------------------------------------------------*/
static bool setInstance(struct IwdgBase *object)
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
uint32_t iwdgGetClock(const struct IwdgBase *timer __attribute__((unused)))
{
  return clockFrequency(InternalLowSpeedOsc);
}
/*----------------------------------------------------------------------------*/
static enum Result wdtInit(void *object, const void *configBase
    __attribute__((unused)))
{
  struct IwdgBase * const timer = object;

  if (setInstance(timer))
  {
    if (STM_RCC->CSR & CSR_IWDGRSTF)
    {
      STM_RCC->CSR |= CSR_RMVF;
      timer->fired = true;
    }
    else
      timer->fired = false;

    return E_OK;
  }
  else
    return E_BUSY;
}
