/*
 * halm/core/cortex/systick_defs.h
 * Copyright (C) 2018 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef HALM_CORE_CORTEX_SYSTICK_DEFS_H_
#define HALM_CORE_CORTEX_SYSTICK_DEFS_H_
/*----------------------------------------------------------------------------*/
#include <xcore/bits.h>
/*----------------------------------------------------------------------------*/
#define TIMER_RESOLUTION                ((1UL << 24) - 1)

/* Enables counter operation */
#define CTRL_ENABLE                     BIT(0)
/* Counting down to zero pends the SysTick handler */
#define CTRL_TICKINT                    BIT(1)
/* Clock source selection: 0 for external clock, 1 for core clock */
#define CTRL_CLKSOURCE                  BIT(2)
/* Counter flag is set when counter counts down to zero */
#define CTRL_COUNTFLAG                  BIT(16)
/*----------------------------------------------------------------------------*/
#endif /* HALM_CORE_CORTEX_SYSTICK_DEFS_H_ */
