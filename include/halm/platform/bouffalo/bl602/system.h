/*
 * halm/platform/bouffalo/bl602/system.h
 * Copyright (C) 2026 xent
 * Project is distributed under the terms of the MIT License
 */

/**
 * @file
 * System configuration functions for i.MX RT106x chips.
 */

#ifndef HALM_PLATFORM_BOUFFALO_SYSTEM_H_
#error This header should not be included directly
#endif

#ifndef HALM_PLATFORM_BOUFFALO_BL602_SYSTEM_H_
#define HALM_PLATFORM_BOUFFALO_BL602_SYSTEM_H_
/*----------------------------------------------------------------------------*/
#include <xcore/helpers.h>
/*----------------------------------------------------------------------------*/
/* Enable or disable clock branches */
enum [[gnu::packed]] SysClockBranch
{
  // TODO
};
/*----------------------------------------------------------------------------*/
BEGIN_DECLS

void sysClockDisable(enum SysClockBranch);
void sysClockEnable(enum SysClockBranch);
bool sysClockStatus(enum SysClockBranch);

END_DECLS
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_BOUFFALO_BL602_SYSTEM_H_ */
