/*
 * pm.c
 * Copyright (C) 2014 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <pm.h>
#include <platform/platform_defs.h>
#include <platform/nxp/lpc43xx/system_defs.h>
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
      LPC_PMC->PD0_SLEEP0_MODE = MODE_POWERDOWN;
      break;

    default:
      break;
  }

  return E_OK;
}
