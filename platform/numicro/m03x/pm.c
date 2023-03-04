/*
 * pm.c
 * Copyright (C) 2023 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/platform/numicro/m03x/clocking_defs.h>
#include <halm/platform/numicro/system.h>
#include <halm/platform/platform_defs.h>
#include <halm/pm.h>
/*----------------------------------------------------------------------------*/
void pmPlatformChangeState(enum PmState);
/*----------------------------------------------------------------------------*/
void pmPlatformChangeState(enum PmState state)
{
  switch (state)
  {
    case PM_SUSPEND:
    case PM_SHUTDOWN:
      sysUnlockReg();
      NM_CLK->PWRCTL |= PWRCTL_PDEN;
      sysLockReg();
      break;

    default:
      break;
  }
}
