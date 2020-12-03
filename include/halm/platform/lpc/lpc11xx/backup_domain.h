/*
 * halm/platform/lpc/lpc11xx/backup_domain.h
 * Copyright (C) 2017 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_LPC_LPC11XX_BACKUP_DOMAIN_H_
#define HALM_PLATFORM_LPC_LPC11XX_BACKUP_DOMAIN_H_
/*----------------------------------------------------------------------------*/
#include <halm/platform/platform_defs.h>
#include <xcore/helpers.h>
#include <stddef.h>
/*----------------------------------------------------------------------------*/
BEGIN_DECLS

static inline void *backupDomainAddress(void)
{
  return (void *)LPC_PMU->GPREG;
}

static inline size_t backupDomainSize(void)
{
  return sizeof(LPC_PMU->GPREG);
}

END_DECLS
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_LPC_LPC11XX_BACKUP_DOMAIN_H_ */
