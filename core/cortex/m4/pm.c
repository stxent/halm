/*
 * pm.c
 * Copyright (C) 2014 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <error.h>
#include <pm.h>
#include <core/cortex/m3/asm.h>
#include <core/cortex/m3/core_defs.h>
#include <core/cortex/m3/pm_defs.h>
/*----------------------------------------------------------------------------*/
enum result pmCoreChangeState(enum pmState);
/*----------------------------------------------------------------------------*/
enum result pmCoreChangeState(enum pmState state)
{
  switch (state)
  {
    case PM_SUSPEND:
    case PM_POWERDOWN:
      SCB->SCR |= SCR_SLEEPDEEP;
      __wfi();
      break;

    case PM_SLEEP:
      SCB->SCR &= ~SCR_SLEEPDEEP;
      __wfi();
      break;

    default:
      break;
  }

  return E_OK;
}
