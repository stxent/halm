/*
 * pm.c
 * Copyright (C) 2014 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <halm/platform/nxp/lpc11exx/system.h>
#include <halm/platform/nxp/lpc11exx/system_defs.h>
#include <halm/platform/platform_defs.h>
#include <halm/pm.h>
/*----------------------------------------------------------------------------*/
void pmPlatformChangeState(enum PmState);
/*----------------------------------------------------------------------------*/
void pmPlatformChangeState(enum PmState state)
{
  static const uint32_t mask =
      ~(PCON_PM_MASK | PCON_PM_SLEEPFLAG | PCON_PM_DPDFLAG);
  uint32_t value;

  switch (state)
  {
    case PM_SLEEP:
      value = PCON_PM_SLEEPFLAG | PCON_PM(PCON_PM_DEFAULT);
      break;

    case PM_SUSPEND:
#if defined(CONFIG_PLATFORM_NXP_PM_PD)
      value = PCON_PM_SLEEPFLAG | PCON_PM(PCON_PM_POWERDOWN);
#else
      value = PCON_PM_SLEEPFLAG | PCON_PM(PCON_PM_DEEPSLEEP);
#endif
      break;

    case PM_SHUTDOWN:
      value = PCON_PM_DPDFLAG | PCON_PM(PCON_PM_DEEPPOWERDOWN);
      break;

    default:
      return;
  }

  LPC_PMU->PCON = (LPC_PMU->PCON & mask) | value;

  if (state == PM_SUSPEND)
  {
    uint32_t config = LPC_SYSCON->PDSLEEPCFG;

    if (sysPowerStatus(PWR_BOD))
      config &= ~(1UL << PWR_BOD);
    if (sysPowerStatus(PWR_WDTOSC))
      config &= ~(1UL << PWR_WDTOSC);

    LPC_SYSCON->PDSLEEPCFG = config;
    LPC_SYSCON->PDAWAKECFG = LPC_SYSCON->PDRUNCFG;
  }
}
