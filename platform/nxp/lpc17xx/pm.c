/*
 * pm.c
 * Copyright (C) 2014 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <halm/platform/nxp/lpc17xx/system_defs.h>
#include <halm/platform/platform_defs.h>
#include <halm/pm.h>
/*----------------------------------------------------------------------------*/
enum result pmPlatformChangeState(enum pmState);
/*----------------------------------------------------------------------------*/
enum result pmPlatformChangeState(enum pmState state)
{
  uint32_t value = LPC_SC->PCON & ~(PCON_PM_MASK | PCON_SMFLAG | PCON_DSFLAG
      | PCON_PDFLAG | PCON_DPDFLAG);

  switch (state)
  {
    case PM_SLEEP:
      value |= PCON_PM(PCON_PM_SLEEP) | PCON_SMFLAG;
      break;

    case PM_SUSPEND:
#if defined(CONFIG_PLATFORM_NXP_PM_PD)
      value |= PCON_PM(PCON_PM_SLEEP) | PCON_DSFLAG;
#elif defined(CONFIG_PLATFORM_NXP_PM_DPD)
      value |= PCON_PM(PCON_PM_POWERDOWN) | PCON_PDFLAG;
#else
      value |= PCON_PM(PCON_PM_DEEP_POWERDOWN) | PCON_DPDFLAG;
#endif

    default:
      return E_OK;
  }

  LPC_SC->PCON = value;
  return E_OK;
}
