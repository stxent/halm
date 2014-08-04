/*
 * pm.c
 * Copyright (C) 2014 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <error.h>
#include <pm.h>
#include <platform/nxp/platform_defs.h>
#include <platform/nxp/lpc13xx/system_defs.h>
/*----------------------------------------------------------------------------*/
enum result pmPlatformChangeState(enum pmState);
/*----------------------------------------------------------------------------*/
enum result pmPlatformChangeState(enum pmState state)
{
  switch (state)
  {
    case PM_SLEEP:
      LPC_PMU->PCON &= ~PCON_DPDEN;
      break;

    case PM_SUSPEND:
      LPC_PMU->PCON |= PCON_DPDEN;
      break;

    default:
      break;
  }

  return E_OK;
}
