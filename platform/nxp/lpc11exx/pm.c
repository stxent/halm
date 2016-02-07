/*
 * pm.c
 * Copyright (C) 2014 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <platform/nxp/lpc11exx/system.h>
#include <platform/nxp/lpc11exx/system_defs.h>
#include <platform/platform_defs.h>
#include <pm.h>
/*----------------------------------------------------------------------------*/
enum result pmPlatformChangeState(enum pmState);
/*----------------------------------------------------------------------------*/
enum result pmPlatformChangeState(enum pmState state)
{
  uint32_t value = LPC_PMU->PCON & ~(PCON_PM_MASK | PCON_PM_SLEEPFLAG
      | PCON_PM_DPDFLAG);

  switch (state)
  {
    case PM_SLEEP:
      value = PCON_PM_SLEEPFLAG | PCON_PM(PCON_PM_DEFAULT);
      break;

    case PM_SUSPEND:
#if defined(CONFIG_PLATFORM_NXP_PM_PD)
      value = PCON_PM_SLEEPFLAG | PCON_PM(PCON_PM_POWERDOWN);
#elif defined(CONFIG_PLATFORM_NXP_PM_DPD)
      value = PCON_PM_DPDFLAG | PCON_PM(PCON_PM_DEEPPOWERDOWN);
#else
      value = PCON_PM_SLEEPFLAG | PCON_PM(PCON_PM_DEEPSLEEP);
#endif
      break;

    default:
      return E_OK;
  }
  LPC_PMU->PCON = value;

#ifndef CONFIG_PLATFORM_NXP_PM_DPD
  if (state == PM_SUSPEND)
  {
    uint32_t config = LPC_SYSCON->PDSLEEPCFG;

    if (sysPowerStatus(PWR_BOD))
      config &= ~BIT(PWR_BOD);
    if (sysPowerStatus(PWR_WDTOSC))
      config &= ~BIT(PWR_WDTOSC);

    LPC_SYSCON->PDSLEEPCFG = config;
    LPC_SYSCON->PDAWAKECFG = LPC_SYSCON->PDRUNCFG;
  }
#endif

  return E_OK;
}
