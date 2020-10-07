/*
 * halm/platform/nxp/lpc43xx/backup_domain.h
 * Copyright (C) 2017 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef HALM_PLATFORM_NXP_LPC43XX_BACKUP_DOMAIN_H_
#define HALM_PLATFORM_NXP_LPC43XX_BACKUP_DOMAIN_H_
/*----------------------------------------------------------------------------*/
#include <halm/platform/platform_defs.h>
#include <xcore/helpers.h>
#include <stddef.h>
/*----------------------------------------------------------------------------*/
BEGIN_DECLS

static inline void *backupDomainAddress(void)
{
  return (void *)LPC_BACKUP->DATA;
}

static inline size_t backupDomainSize(void)
{
  return sizeof(LPC_BACKUP->DATA);
}

END_DECLS
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_NXP_LPC43XX_BACKUP_DOMAIN_H_ */
