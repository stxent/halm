/*
 * pm.c
 * Copyright (C) 2014 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <pm.h>
#include <platform/platform_defs.h>
#include <platform/nxp/lpc17xx/system_defs.h>
/*----------------------------------------------------------------------------*/
enum result pmPlatformChangeState(enum pmState);
/*----------------------------------------------------------------------------*/
enum result pmPlatformChangeState(enum pmState state)
{
  switch (state)
  {
    case PM_SLEEP:
      LPC_SC->PCON = (LPC_SC->PCON & PCON_PM_MASK) | PCON_PM(PCON_PM_SLEEP);
      break;

    case PM_SUSPEND:
      LPC_SC->PCON = (LPC_SC->PCON & PCON_PM_MASK) | PCON_PM(PCON_PM_POWERDOWN);
      break;

    default:
      break;
  }

  return E_OK;
}
