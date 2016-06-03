/*
 * halm/core/cortex/pm_defs.h
 * Copyright (C) 2014 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef HALM_CORE_CORTEX_PM_DEFS_H_
#define HALM_CORE_CORTEX_PM_DEFS_H_
/*----------------------------------------------------------------------------*/
#include <xcore/bits.h>
/*------------------System Control Register-----------------------------------*/
#define SCR_SEVONPEND                   BIT(4)
#define SCR_SLEEPDEEP                   BIT(2)
#define SCR_SLEEPONEXIT                 BIT(1)
/*----------------------------------------------------------------------------*/
#endif /* HALM_CORE_CORTEX_PM_DEFS_H_ */
