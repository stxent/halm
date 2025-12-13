/*
 * pm.c
 * Copyright (C) 2025 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/platform/lpc/lpc82x/system_defs.h>
#include <halm/platform/lpc/system.h>
#include <halm/platform/platform_defs.h>
#include <halm/pm.h>
/*----------------------------------------------------------------------------*/
void pmPlatformChangeState(enum PmState);
/*----------------------------------------------------------------------------*/
void pmPlatformChangeState(enum PmState state)
{
  static const uint32_t mask = ~(PCON_NODPD | PCON_SLEEPFLAG | PCON_DPDFLAG);
  uint32_t value;

  switch (state)
  {
    case PM_SLEEP:
      value = PCON_SLEEPFLAG | PCON_PM(PCON_PM_DEFAULT);
      break;

    case PM_SUSPEND:
#ifdef CONFIG_PLATFORM_LPC_PM_PD
      value = PCON_SLEEPFLAG | PCON_PM(PCON_PM_POWERDOWN);
#else
      value = PCON_SLEEPFLAG | PCON_PM(PCON_PM_DEEPSLEEP);
#endif
      break;

    case PM_SHUTDOWN:
      value = PCON_DPDFLAG | PCON_PM(PCON_PM_DEEPPOWERDOWN);
      break;

    default:
      return;
  }

  LPC_PMU->PCON = (LPC_PMU->PCON & mask) | value;

  if (state == PM_SUSPEND)
  {
    uint32_t config = LPC_SYSCON->PDSLEEPCFG;

#ifndef CONFIG_PLATFORM_LPC_PM_DISABLE_BOD
    config &= ~(1UL << PWR_BOD);
#endif
#ifndef CONFIG_PLATFORM_LPC_PM_DISABLE_WDT
    config &= ~(1UL << PWR_WDTOSC);
#endif

    LPC_SYSCON->PDSLEEPCFG = config;
    LPC_SYSCON->PDAWAKECFG = LPC_SYSCON->PDRUNCFG;
  }
}
