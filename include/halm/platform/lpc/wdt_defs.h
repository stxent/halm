/*
 * halm/platform/lpc/wdt_defs.h
 * Copyright (C) 2014 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_LPC_WDT_DEFS_H_
#define HALM_PLATFORM_LPC_WDT_DEFS_H_
/*----------------------------------------------------------------------------*/
#include <xcore/bits.h>
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
#define CLKSEL_WDSEL_MASK               BIT_FIELD(MASK(2), 0)
#define CLKSEL_WDSEL(value)             BIT_FIELD((value), 0)
#define CLKSEL_WDSEL_VALUE(reg)         FIELD_VALUE((reg), CLKSEL_WDSEL_MASK, 0)
#define CLKSEL_LOCK                     BIT(31)
/*------------------Timer Warning Interrupt register--------------------------*/
#define WARNINT_MAX                     MASK(10)
/*------------------Timer Window register-------------------------------------*/
#define WINDOW_MAX                      MASK(24)
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_LPC_WDT_DEFS_H_ */
