/*
 * pm.c
 * Copyright (C) 2014 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <xcore/core/cortex/asm.h>
#include <halm/core/core_defs.h>
#include <halm/core/cortex/pm_defs.h>
#include <halm/pm.h>
/*----------------------------------------------------------------------------*/
void pmCoreChangeState(enum pmState);
/*----------------------------------------------------------------------------*/
void pmCoreChangeState(enum pmState state)
{
  if (state != PM_ACTIVE)
  {
    if (state == PM_SLEEP)
    {
      SCB->SCR &= ~SCR_SLEEPDEEP;
    }
    else
    {
      SCB->SCR |= SCR_SLEEPDEEP;
    }

    __dsb();
    __wfi();
  }
}
