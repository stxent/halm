/*
 * halm/platform/stm32/stm32f1xx/backup_domain.h
 * Copyright (C) 2026 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_STM32_BACKUP_DOMAIN_H_
#error This header should not be included directly
#endif

#ifndef HALM_PLATFORM_STM32_STM32F1XX_BACKUP_DOMAIN_H_
#define HALM_PLATFORM_STM32_STM32F1XX_BACKUP_DOMAIN_H_
/*----------------------------------------------------------------------------*/
#include <halm/platform/platform_defs.h>
#include <xcore/helpers.h>
#include <stddef.h>
/*----------------------------------------------------------------------------*/
BEGIN_DECLS

static inline void *backupDomainAddress(void)
{
  // TODO STM32F1 Add lower part of backup registers
  return (void *)STM_BKP->DRH;
}

static inline size_t backupDomainSize(void)
{
  return sizeof(STM_BKP->DRH);
}

END_DECLS
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_STM32_STM32F1XX_BACKUP_DOMAIN_H_ */
