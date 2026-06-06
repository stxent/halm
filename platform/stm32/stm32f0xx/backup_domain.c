/*
 * backup_domain.c
 * Copyright (C) 2026 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/platform/stm32/backup_domain.h>
#include <halm/platform/stm32/stm32f0xx/system_defs.h>
#include <halm/platform/stm32/system.h>
/*----------------------------------------------------------------------------*/
void backupDomainDisable(void)
{
  sysClockDisable(CLK_PWR);
}
/*----------------------------------------------------------------------------*/
void backupDomainEnable(void)
{
  if (!sysClockStatus(CLK_PWR))
  {
    sysClockEnable(CLK_PWR);
    sysResetPulse(RST_PWR);
  }
}
/*----------------------------------------------------------------------------*/
void backupDomainLock(void)
{
  STM_PWR->CR &= ~PWR_CR_DBP;
}
/*----------------------------------------------------------------------------*/
void backupDomainUnlock(void)
{
  STM_PWR->CR |= PWR_CR_DBP;
}
