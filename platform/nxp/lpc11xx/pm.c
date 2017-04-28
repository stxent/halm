/*
 * pm.c
 * Copyright (C) 2014 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <halm/platform/nxp/lpc11xx/system.h>
#include <halm/platform/nxp/lpc11xx/system_defs.h>
#include <halm/platform/platform_defs.h>
#include <halm/pm.h>
/*----------------------------------------------------------------------------*/
void pmPlatformChangeState(enum pmState);
/*----------------------------------------------------------------------------*/
void pmPlatformChangeState(enum pmState state)
{
  static const uint32_t mask = ~(PCON_SLEEPFLAG | PCON_DPDFLAG | PCON_DPDEN);
  uint32_t value;

  switch (state)
  {
    case PM_SLEEP:
      value = PCON_SLEEPFLAG;
      break;

    case PM_SUSPEND:
      value = PCON_SLEEPFLAG;
      LPC_SYSCON->PDAWAKECFG = LPC_SYSCON->PDRUNCFG;
      break;

    case PM_SHUTDOWN:
      value = PCON_DPDEN | PCON_SLEEPFLAG | PCON_DPDFLAG;
      break;

    default:
      return;
  }

  LPC_PMU->PCON = (LPC_PMU->PCON & mask) | value;

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
}
