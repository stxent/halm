/*
 * pm.c
 * Copyright (C) 2014 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/platform/lpc/lpc43xx/system_defs.h>
#include <halm/platform/platform_defs.h>
#include <halm/pm.h>
/*----------------------------------------------------------------------------*/
void pmPlatformChangeState(enum PmState);
/*----------------------------------------------------------------------------*/
void pmPlatformChangeState(enum PmState state)
{
  switch (state)
  {
    case PM_ACTIVE:
      /* Enable clocks */
      LPC_CCU2->PM = 0;
      LPC_CCU1->PM = 0;
      break;

    case PM_SLEEP:
      break;

    case PM_SUSPEND:
      /* Workaround for LPC43xx Errata */
      LPC_CREG->CREG1 =
          (LPC_CREG->CREG1 & ~CREG1_RESERVED_16) | CREG1_RESERVED_17;

      /* Disable clocks */
      LPC_CCU1->PM = PM_PD;
      LPC_CCU2->PM = PM_PD;

#ifdef CONFIG_PLATFORM_LPC_PM_PD
      LPC_PMC->PD0_SLEEP0_MODE = MODE_POWERDOWN;
#elifdef CONFIG_PLATFORM_LPC_PM_PD_M0SUB
      LPC_PMC->PD0_SLEEP0_MODE = MODE_POWERDOWN_M0SUB;
#else
      LPC_PMC->PD0_SLEEP0_MODE = MODE_DEEP_SLEEP;
#endif

      LPC_PMC->PD0_SLEEP0_HW_ENA = ENA_EVENT0;
      break;

    case PM_SHUTDOWN:
      LPC_PMC->PD0_SLEEP0_MODE = MODE_DEEP_POWERDOWN;
      LPC_PMC->PD0_SLEEP0_HW_ENA = ENA_EVENT0;
      break;

    default:
      break;
  }
}
