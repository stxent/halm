/*
 * pm.c
 * Copyright (C) 2014 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/core/core_defs.h>
#include <halm/core/cortex/pm_defs.h>
#include <halm/pm.h>
#include <xcore/asm.h>
/*----------------------------------------------------------------------------*/
void pmCoreChangeState(enum PmState);
/*----------------------------------------------------------------------------*/
void pmCoreChangeState(enum PmState state)
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
