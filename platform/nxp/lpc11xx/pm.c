/*
 * pm.c
 * Copyright (C) 2014 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <platform/nxp/lpc11xx/system.h>
#include <platform/nxp/lpc11xx/system_defs.h>
#include <platform/platform_defs.h>
#include <pm.h>
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
      LPC_SYSCON->PDAWAKECFG = LPC_SYSCON->PDRUNCFG;
#endif
      break;

    default:
      return E_OK;
  }
  LPC_PMU->PCON = value;

#ifndef CONFIG_PLATFORM_NXP_PM_DPD
  if (state == PM_SUSPEND)
  {
    /* Prepare magic number */
    uint32_t config = 0x000018FF;

    if (sysPowerStatus(PWR_BOD))
      config &= ~(1UL << PWR_BOD);
    if (sysPowerStatus(PWR_WDTOSC))
      config &= ~(1UL << PWR_WDTOSC);

    LPC_SYSCON->PDSLEEPCFG = config;
  }
#endif

  return E_OK;
}
