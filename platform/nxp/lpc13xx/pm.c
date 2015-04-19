/*
 * pm.c
 * Copyright (C) 2014 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <pm.h>
#include <platform/platform_defs.h>
#include <platform/nxp/lpc13xx/system.h>
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

#ifndef CONFIG_PLATFORM_NXP_PM_DPD
  if (state == PM_SUSPEND)
  {
    /* Prepare magic number */
    uint32_t number = 0x000000FF;

    if (sysPowerStatus(PWR_BOD))
      number &= ~BIT(PWR_BOD);
    if (sysPowerStatus(PWR_WDTOSC))
      number &= ~BIT(PWR_WDTOSC);

    /* Detect chip version */
    if (LPC_SYSCON->DEVICE_ID >> 24 == 0x18)
    {
      /* LPC1300L */
      number |= 0x1800;
    }
    else
    {
      /* LPC1300 */
      number |= 0x0F00;
    }

    LPC_SYSCON->PDSLEEPCFG = number;
  }
#endif

  return E_OK;
}
