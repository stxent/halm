/*
 * pm.c
 * Copyright (C) 2014 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <platform/nxp/lpc43xx/system_defs.h>
#include <platform/platform_defs.h>
#include <pm.h>
/*----------------------------------------------------------------------------*/
enum result pmPlatformChangeState(enum pmState);
/*----------------------------------------------------------------------------*/
enum result pmPlatformChangeState(enum pmState state)
{
  switch (state)
  {
    case PM_SLEEP:
      LPC_PMC->PD0_SLEEP0_HW_ENA &= ~ENA_EVENT0;
      break;

    case PM_SUSPEND:
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
