/*
 * platform/nxp/lpc11exx/system_defs.h
 * Copyright (C) 2014 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef PLATFORM_NXP_LPC11EXX_SYSTEM_DEFS_H_
#define PLATFORM_NXP_LPC11EXX_SYSTEM_DEFS_H_
/*----------------------------------------------------------------------------*/
#include <bits.h>
/*------------------Flash Configuration register------------------------------*/
#define FLASHCFG_FLASHTIM_MASK          BIT_FIELD(MASK(2), 0)
#define FLASHCFG_FLASHTIM(value)        BIT_FIELD((value), 0)
#define FLASHCFG_FLASHTIM_VALUE(reg) \
    FIELD_VALUE((reg), FLASHCFG_FLASHTIM_MASK, 0)
/*------------------Peripheral Reset Control register-------------------------*/
#define PRESETCTRL_SSP0                 BIT(0)
#define PRESETCTRL_I2C                  BIT(1)
#define PRESETCTRL_SSP1                 BIT(2)
/*------------------Power Control register------------------------------------*/
#define PCON_PM_MASK                    BIT_FIELD(MASK(3), 0)
#define PCON_PM(value)                  BIT_FIELD((value), 0)
#define PCON_PM_VALUE(reg)              FIELD_VALUE((reg), PCON_PM_MASK, 0)
#define PCON_PM_DEFAULT                 0
#define PCON_PM_DEEPSLEEP               1
#define PCON_PM_POWERDOWN               2
#define PCON_PM_DEEPPOWERDOWN           3

#define PCON_PM_NODPD                   BIT(3)
#define PCON_PM_SLEEPFLAG               BIT(8)
#define PCON_PM_DPDFLAG                 BIT(11)
/*------------------Interrupt wake-up enable registers------------------------*/
#define STARTERP0_PINT(channel)         BIT(channel)
#define STARTERP1_WWDTINT               BIT(12)
#define STARTERP1_BODINT                BIT(13)
#define STARTERP1_GPIOINT(channel)      BIT((channel) + 20)
/*----------------------------------------------------------------------------*/
#endif /* PLATFORM_NXP_LPC11EXX_SYSTEM_DEFS_H_ */
