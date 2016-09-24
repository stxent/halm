/*
 * pm.c
 * Copyright (C) 2016 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <halm/platform/stm/stm32f1xx/system.h>
#include <halm/platform/stm/stm32f1xx/system_defs.h>
#include <halm/platform/platform_defs.h>
#include <halm/pm.h>
/*----------------------------------------------------------------------------*/
enum result pmPlatformChangeState(enum pmState);
/*----------------------------------------------------------------------------*/
enum result pmPlatformChangeState(enum pmState state)
{
  uint32_t value = STM_PWR->CR & ~(PWR_CR_LPDS | PWR_CR_PDDS) | PWR_CR_CWUF;

  switch (state)
  {
    case PM_SLEEP:
      break;

    case PM_SUSPEND:
#ifdef CONFIG_PLATFORM_STM_PM_STOP
      value |= PWR_CR_LPDS;
#else
      value |= PWR_CR_PDDS | PWR_CR_CSBF;
#endif
      break;

    default:
      return E_OK;
  }

  STM_PWR->CR = value;
  return E_OK;
}
