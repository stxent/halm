/*
 * halm/platform/stm32/stm32f4xx/backup_domain.h
 * Copyright (C) 2026 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_STM32_BACKUP_DOMAIN_H_
#error This header should not be included directly
#endif

#ifndef HALM_PLATFORM_STM32_STM32F4XX_BACKUP_DOMAIN_H_
#define HALM_PLATFORM_STM32_STM32F4XX_BACKUP_DOMAIN_H_
/*----------------------------------------------------------------------------*/
#include <halm/platform/platform_defs.h>
#include <xcore/helpers.h>
#include <stddef.h>
/*----------------------------------------------------------------------------*/
BEGIN_DECLS

void backupDomainDisable(void);
void backupDomainEnable(void);
void backupDomainLock(void);
void backupDomainUnlock(void);

END_DECLS
/*----------------------------------------------------------------------------*/
BEGIN_DECLS

static inline void *backupDomainAddress(void)
{
  return (void *)STM_RTC->BKPR;
}

static inline size_t backupDomainSize(void)
{
  return sizeof(STM_RTC->BKPR);
}

END_DECLS
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_STM32_STM32F4XX_BACKUP_DOMAIN_H_ */
