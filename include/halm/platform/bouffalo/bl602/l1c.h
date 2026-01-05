/*
 * halm/platform/bouffalo/bl602/l1c.h
 * Copyright (C) 2026 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_BOUFFALO_BL602_L1C_H_
#define HALM_PLATFORM_BOUFFALO_BL602_L1C_H_
/*----------------------------------------------------------------------------*/
#include <xcore/helpers.h>
/*----------------------------------------------------------------------------*/
BEGIN_DECLS

void cacheControlAccess2T(bool);
void cacheDisable(void);

END_DECLS
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_BOUFFALO_BL602_L1C_H_ */
