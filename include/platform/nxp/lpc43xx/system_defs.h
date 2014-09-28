/*
 * platform/nxp/lpc43xx/system_defs.h
 * Copyright (C) 2014 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef PLATFORM_NXP_LPC43XX_SYSTEM_DEFS_H_
#define PLATFORM_NXP_LPC43XX_SYSTEM_DEFS_H_
/*----------------------------------------------------------------------------*/
#include <bits.h>
/*------------------Branch clock configuration registers----------------------*/
#define CFG_RUN                         BIT(0)
#define CFG_AUTO                        BIT(1)
#define CFG_WAKEUP                      BIT(2)
/*------------------EMC clock divider configuration register------------------*/
#define CFG_DIV(value)                  BIT_FIELD((value), 5)
#define CFG_DIV_MASK                    BIT_FIELD(MASK(3), 27)
#define CFG_DIV_VALUE(reg)              FIELD_VALUE((reg), CFG_DIV_MASK, 27)
/*------------------Branch clock status registers-----------------------------*/
#define STAT_RUN                        BIT(0)
#define STAT_AUTO                       BIT(1)
#define STAT_WAKEUP                     BIT(2)
#define STAT_RUN_N                      BIT(5)
/*----------------------------------------------------------------------------*/
#endif /* PLATFORM_NXP_LPC43XX_SYSTEM_DEFS_H_ */
