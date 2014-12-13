/*
 * wwdt_defs.h
 * Copyright (C) 2014 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef PLATFORM_NXP_WWDT_DEFS_H_
#define PLATFORM_NXP_WWDT_DEFS_H_
/*----------------------------------------------------------------------------*/
#include <bits.h>
/*------------------Feed register---------------------------------------------*/
#define FEED_FIRST                      0xAA
#define FEED_SECOND                     0x55
/*------------------Mode register---------------------------------------------*/
/* Enable bit */
#define MOD_WDEN                        BIT(0)
/* Reset enable bit */
#define MOD_WDRESET                     BIT(1)
/* Time-out flag */
#define MOD_WDTOF                       BIT(2)
/* Interrupt flag */
#define MOD_WDINT                       BIT(3)
/* Update mode */
#define MOD_WDPROTECT                   BIT(4)
/* Prevent disabling of the watchdog oscillator */
#define MOD_LOCK                        BIT(5)
/*------------------Watchdog Clock Select register----------------------------*/
/* When set module is clocked from watchdog oscillator */
#define CLKSEL_WDOSC                    BIT(0)
#define CLKSEL_LOCK                     BIT(31)
/*----------------------------------------------------------------------------*/
#endif /* PLATFORM_NXP_WWDT_DEFS_H_ */
