/*
 * pm.c
 * Copyright (C) 2016 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/platform/stm32/stm32f1xx/system.h>
#include <halm/platform/stm32/stm32f1xx/system_defs.h>
#include <halm/platform/platform_defs.h>
#include <halm/pm.h>
/*----------------------------------------------------------------------------*/
void pmPlatformChangeState(enum PmState);
/*----------------------------------------------------------------------------*/
void pmPlatformChangeState(enum PmState state)
{
  uint32_t value = (STM_PWR->CR & ~(PWR_CR_LPDS | PWR_CR_PDDS)) | PWR_CR_CWUF;

  switch (state)
  {
    case PM_SLEEP:
      break;

    case PM_SUSPEND:
      value |= PWR_CR_LPDS;
      break;

    case PM_SHUTDOWN:
      value |= PWR_CR_PDDS | PWR_CR_CSBF;
      break;

    default:
      return;
  }

  STM_PWR->CR = value;
}
