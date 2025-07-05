/*
 * halm/platform/lpc/rit_defs.h
 * Copyright (C) 2018 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_LPC_RIT_DEFS_H_
#define HALM_PLATFORM_LPC_RIT_DEFS_H_
/*----------------------------------------------------------------------------*/
#include <xcore/bits.h>
/*----------------------------------------------------------------------------*/
#define TIMER_RESOLUTION                0xFFFFFFFFUL
/*------------------Control register------------------------------------------*/
#define CTRL_RITINT                     BIT(0)
#define CTRL_RITENCLR                   BIT(1)
#define CTRL_RITENBR                    BIT(2)
#define CTRL_RITEN                      BIT(3)
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_LPC_RIT_DEFS_H_ */
