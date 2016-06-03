/*
 * pm.c
 * Copyright (C) 2014 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <halm/platform/nxp/lpc43xx/system_defs.h>
#include <halm/platform/platform_defs.h>
#include <halm/pm.h>
/*----------------------------------------------------------------------------*/
enum result pmPlatformChangeState(enum pmState);
/*----------------------------------------------------------------------------*/
enum result pmPlatformChangeState(enum pmState state)
{
  switch (state)
  {
    case PM_ACTIVE:
      /* Enable clocks */
      LPC_CCU2->PM = 0;
      LPC_CCU1->PM = 0;
      break;

    case PM_SLEEP:
      LPC_PMC->PD0_SLEEP0_HW_ENA &= ~ENA_EVENT0;
      break;

    case PM_SUSPEND:
      /* Disable clocks */
      LPC_CCU1->PM = PM_PD;
      LPC_CCU2->PM = PM_PD;

      LPC_PMC->PD0_SLEEP0_HW_ENA |= ENA_EVENT0;

#if defined(CONFIG_PLATFORM_NXP_PM_PD)
      /* TODO Power-down mode with M0 SRAM maintained */
      LPC_PMC->PD0_SLEEP0_MODE = MODE_POWERDOWN;
#elif defined(CONFIG_PLATFORM_NXP_PM_DPD)
      LPC_PMC->PD0_SLEEP0_MODE = MODE_DEEP_POWERDOWN;
#else
      LPC_PMC->PD0_SLEEP0_MODE = MODE_DEEP_SLEEP;
#endif
      break;

    default:
      break;
  }

  return E_OK;
}
