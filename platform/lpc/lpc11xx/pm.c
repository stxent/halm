/*
 * pm.c
 * Copyright (C) 2014 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/platform/lpc/lpc11xx/system_defs.h>
#include <halm/platform/lpc/system.h>
#include <halm/platform/platform_defs.h>
#include <halm/pm.h>
/*----------------------------------------------------------------------------*/
void pmPlatformChangeState(enum PmState);
/*----------------------------------------------------------------------------*/
void pmPlatformChangeState(enum PmState state)
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
    uint32_t config = 0x000018FFUL;

#ifndef CONFIG_PLATFORM_LPC_PM_DISABLE_BOD
    config &= ~(1UL << PWR_BOD);
#endif
#ifndef CONFIG_PLATFORM_LPC_PM_DISABLE_WDT
    config &= ~(1UL << PWR_WDTOSC);
#endif

    LPC_SYSCON->PDSLEEPCFG = config;
  }
}
