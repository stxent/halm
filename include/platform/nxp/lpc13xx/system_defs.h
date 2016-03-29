/*
 * platform/nxp/lpc13xx/system_defs.h
 * Copyright (C) 2013 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef HALM_PLATFORM_NXP_LPC13XX_SYSTEM_DEFS_H_
#define HALM_PLATFORM_NXP_LPC13XX_SYSTEM_DEFS_H_
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
#define PCON_DPDEN                      BIT(1)
#define PCON_SLEEPFLAG                  BIT(8)
#define PCON_DPDFLAG                    BIT(11)
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_NXP_LPC13XX_SYSTEM_DEFS_H_ */
