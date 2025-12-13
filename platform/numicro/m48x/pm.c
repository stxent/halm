/*
 * pm.c
 * Copyright (C) 2023 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/platform/numicro/m48x/clocking_defs.h>
#include <halm/platform/numicro/system.h>
#include <halm/platform/platform_defs.h>
#include <halm/pm.h>
/*----------------------------------------------------------------------------*/
void pmPlatformChangeState(enum PmState);

#ifndef CONFIG_PLATFORM_NUMICRO_PM_STANDBY
static void enterPowerDownMode(void);
#else
static void enterStandbyMode(void);
#endif
/*----------------------------------------------------------------------------*/
#ifndef CONFIG_PLATFORM_NUMICRO_PM_STANDBY
static void enterPowerDownMode(void)
{
  uint32_t pmuctl =
      NM_CLK->PMUCTL & ~(PMUCTL_PDMSEL_MASK | PMUCTL_SRETSEL_MASK);

#  ifdef CONFIG_PLATFORM_NUMICRO_PM_FWPD
  pmuctl |= PMUCTL_PDMSEL(PDMSEL_FAST_WAKEUP_PD);
#  elifdef CONFIG_PLATFORM_NUMICRO_PM_LLPD
  pmuctl |= PMUCTL_PDMSEL(PDMSEL_LOW_LEAKAGE_PD);
#  else
  pmuctl |= PMUCTL_PDMSEL(PDMSEL_PD);
#  endif

  sysUnlockReg();
  NM_CLK->PMUCTL = pmuctl;
  NM_CLK->PWRCTL |= PWRCTL_PDEN;
  sysLockReg();
}
#endif
/*----------------------------------------------------------------------------*/
#ifdef CONFIG_PLATFORM_NUMICRO_PM_STANDBY
static void enterStandbyMode(void)
{
  uint32_t pmuctl =
      NM_CLK->PMUCTL & ~(PMUCTL_PDMSEL_MASK | PMUCTL_SRETSEL_MASK);

#  ifdef CONFIG_PLATFORM_NUMICRO_PM_SRAM_16K
  pmuctl |= PMUCTL_PDMSEL(PDMSEL_STANDBY_PD_0) | PMUCTL_SRETSEL(SRETSEL_16K);
#  elifdef CONFIG_PLATFORM_NUMICRO_PM_SRAM_32K
  pmuctl |= PMUCTL_PDMSEL(PDMSEL_STANDBY_PD_0) | PMUCTL_SRETSEL(SRETSEL_32K);
#  elifdef CONFIG_PLATFORM_NUMICRO_PM_SRAM_64K
  pmuctl |= PMUCTL_PDMSEL(PDMSEL_STANDBY_PD_0) | PMUCTL_SRETSEL(SRETSEL_64K);
#  elifdef CONFIG_PLATFORM_NUMICRO_PM_SRAM_128K
  pmuctl |= PMUCTL_PDMSEL(PDMSEL_STANDBY_PD_0) | PMUCTL_SRETSEL(SRETSEL_128K);
#  else
  pmuctl |= PMUCTL_PDMSEL(PDMSEL_STANDBY_PD_1);
#  endif

  sysUnlockReg();
  NM_CLK->PMUCTL = pmuctl;
  NM_CLK->PWRCTL |= PWRCTL_PDEN;
  sysLockReg();
}
#endif
/*----------------------------------------------------------------------------*/
void pmPlatformChangeState(enum PmState state)
{
  switch (state)
  {
    case PM_SUSPEND:
#ifndef CONFIG_PLATFORM_NUMICRO_PM_STANDBY
      enterPowerDownMode();
#else
      enterStandbyMode();
#endif
      break;

    case PM_SHUTDOWN:
      sysUnlockReg();
      NM_CLK->PMUCTL = (NM_CLK->PMUCTL & ~PMUCTL_PDMSEL_MASK)
          | PMUCTL_PDMSEL(PDMSEL_DEEP_PD);
      NM_CLK->PWRCTL |= PWRCTL_PDEN;
      sysLockReg();
      break;

    default:
      break;
  }
}
