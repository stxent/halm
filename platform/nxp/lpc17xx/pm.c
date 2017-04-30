/*
 * pm.c
 * Copyright (C) 2014 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <halm/platform/nxp/lpc17xx/system_defs.h>
#include <halm/platform/platform_defs.h>
#include <halm/pm.h>
/*----------------------------------------------------------------------------*/
void pmPlatformChangeState(enum PmState);
/*----------------------------------------------------------------------------*/
void pmPlatformChangeState(enum PmState state)
{
  static const uint32_t mask =
      ~(PCON_PM_MASK | PCON_SMFLAG | PCON_DSFLAG | PCON_PDFLAG | PCON_DPDFLAG);
  uint32_t value;

  switch (state)
  {
    case PM_SLEEP:
      value = PCON_PM(PCON_PM_SLEEP) | PCON_SMFLAG;
      break;

    case PM_SUSPEND:
#if defined(CONFIG_PLATFORM_NXP_PM_PD)
      value = PCON_PM(PCON_PM_POWERDOWN) | PCON_PDFLAG;
#else
      value = PCON_PM(PCON_PM_SLEEP) | PCON_DSFLAG;
#endif
      break;

    case PM_SHUTDOWN:
      value = PCON_PM(PCON_PM_DEEP_POWERDOWN) | PCON_DPDFLAG;
      break;

    default:
      return;
  }

  LPC_SC->PCON = (LPC_SC->PCON & mask) | value;
}
