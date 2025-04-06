/*
 * halm/platform/lpc/lpc11xx/system_defs.h
 * Copyright (C) 2013 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_LPC_LPC11XX_SYSTEM_DEFS_H_
#define HALM_PLATFORM_LPC_LPC11XX_SYSTEM_DEFS_H_
/*----------------------------------------------------------------------------*/
#include <xcore/bits.h>
/*------------------Flash Configuration register------------------------------*/
#define FLASHCFG_FLASHTIM_MASK          BIT_FIELD(MASK(2), 0)
#define FLASHCFG_FLASHTIM(value)        BIT_FIELD((value), 0)
#define FLASHCFG_FLASHTIM_VALUE(reg) \
    FIELD_VALUE((reg), FLASHCFG_FLASHTIM_MASK, 0)
/*------------------System Memory Remap register------------------------------*/
#define SYSMEMREMAP_MAP_MASK            BIT_FIELD(MASK(2), 0)
#define SYSMEMREMAP_MAP(value)          BIT_FIELD((value), 0)
#define SYSMEMREMAP_MAP_VALUE(reg) \
    FIELD_VALUE((reg), SYSMEMREMAP_MAP_MASK, 0)
/*------------------Peripheral Reset Control register-------------------------*/
#define PRESETCTRL_SSP0                 BIT(0)
#define PRESETCTRL_I2C                  BIT(1)
#define PRESETCTRL_SSP1                 BIT(2)
#define PRESETCTRL_CAN                  BIT(3)
/*------------------Power Control register------------------------------------*/
#define PCON_DPDEN                      BIT(1)
#define PCON_SLEEPFLAG                  BIT(8)
#define PCON_DPDFLAG                    BIT(11)
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_LPC_LPC11XX_SYSTEM_DEFS_H_ */
