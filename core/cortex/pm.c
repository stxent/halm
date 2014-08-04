/*
 * pm.c
 * Copyright (C) 2014 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <error.h>
#include <pm.h>
#include <core/cortex/asm.h>
#include <core/cortex/core_defs.h>
#include <core/cortex/pm_defs.h>
/*----------------------------------------------------------------------------*/
enum result pmCoreChangeState(enum pmState);
/*----------------------------------------------------------------------------*/
enum result pmCoreChangeState(enum pmState state)
{
  switch (state)
  {
    case PM_SUSPEND:
      SCB->SCR |= SCR_SLEEPDEEP;
      break;

    case PM_SLEEP:
      SCB->SCR &= ~SCR_SLEEPDEEP;
      break;

    default:
      return E_OK;
  }

  __dsb();
  __wfi();

  return E_OK;
}
