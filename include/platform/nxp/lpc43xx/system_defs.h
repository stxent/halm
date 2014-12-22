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
/*------------------Flash Configuration registers-----------------------------*/
#define FLASHCFG_FLASHTIM_MASK          BIT_FIELD(MASK(4), 12)
#define FLASHCFG_FLASHTIM(value)        BIT_FIELD((value), 12)
#define FLASHCFG_FLASHTIM_VALUE(reg) \
    FIELD_VALUE((reg), FLASHCFG_FLASHTIM_MASK, 12)
#define FLASHCFG_POW                    BIT(31)
/*------------------Branch clock status registers-----------------------------*/
#define STAT_RUN                        BIT(0)
#define STAT_AUTO                       BIT(1)
#define STAT_WAKEUP                     BIT(2)
#define STAT_RUN_N                      BIT(5)
/*------------------Hardware sleep event enable register----------------------*/
#define ENA_EVENT0                      BIT(0)
#define ENA_EVENT1                      BIT(1)
/*------------------Power-down modes register---------------------------------*/
#define MODE_DEEP_SLEEP                 0x003000AA
#define MODE_POWERDOWN                  0x0030FCBA
#define MODE_POWERDOWN_M0SUB            0x00303CBA
#define MODE_DEEP_POWERDOWN             0x0030FF7F
/*----------------------------------------------------------------------------*/
#endif /* PLATFORM_NXP_LPC43XX_SYSTEM_DEFS_H_ */
