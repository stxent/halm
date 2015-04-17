/*
 * pm.c
 * Copyright (C) 2014 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <pm.h>
#include <platform/platform_defs.h>
#include <platform/nxp/lpc13xx/system_defs.h>
/*----------------------------------------------------------------------------*/
enum result pmPlatformChangeState(enum pmState);
/*----------------------------------------------------------------------------*/
enum result pmPlatformChangeState(enum pmState state)
{
  uint32_t value = LPC_PMU->PCON & ~(PCON_SLEEPFLAG | PCON_DPDFLAG);

  switch (state)
  {
    case PM_SLEEP:
      value = (value & ~PCON_DPDEN) | PCON_SLEEPFLAG;
      break;

    case PM_SUSPEND:
#ifdef CONFIG_PLATFORM_NXP_PM_DPD
      value |= PCON_DPDEN | PCON_SLEEPFLAG | PCON_DPDFLAG;
#else
      value = (value & ~PCON_DPDEN) | PCON_SLEEPFLAG;
#endif
      break;

    default:
      return E_OK;
  }

  LPC_PMU->PCON = value;
  return E_OK;
}
