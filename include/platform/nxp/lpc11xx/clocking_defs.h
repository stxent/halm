/*
 * platform/nxp/lpc11xx/clocking_defs.h
 * Copyright (C) 2013 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef CLOCKING_DEFS_
#define CLOCKING_DEFS_
/*----------------------------------------------------------------------------*/
#include <macro.h>
/*------------------Main Clock Source Select Register-------------------------*/
#define MAINCLKSEL_MASK                 BIT_FIELD(0x03, 0)
#define MAINCLKSEL_IRC                  BIT_FIELD(0, 0)
#define MAINCLKSEL_PLL_INPUT            BIT_FIELD(1, 0)
#define MAINCLKSEL_WDT                  BIT_FIELD(2, 0)
#define MAINCLKSEL_PLL_OUTPUT           BIT_FIELD(3, 0)
/*------------------System Oscillator Control Register------------------------*/
#define SYSOSCCTRL_BYPASS               BIT(0)
#define SYSOSCCTRL_FREQRANGE            BIT(1) /* Set for 15 - 25 MHz range */
/*------------------PLL Clock Source Select Registers-------------------------*/
#define PLLCLKSEL_MASK                  BIT_FIELD(0x03, 0)
#define PLLCLKSEL_IRC                   BIT_FIELD(0, 0)
#define PLLCLKSEL_SYSOSC                BIT_FIELD(1, 0)
/*------------------PLL Control Registers-------------------------------------*/
#define PLLCTRL_MSEL_MASK               BIT_FIELD(0x1F, 0)
#define PLLCTRL_MSEL(value)             BIT_FIELD((value), 0)
#define PLLCTRL_PSEL_MASK               BIT_FIELD(0x03, 5)
#define PLLCTRL_PSEL(value)             BIT_FIELD((value), 5)
/*------------------PLL Status Registers--------------------------------------*/
#define PLLSTAT_LOCK                    BIT(0)
/*----------------------------------------------------------------------------*/
#endif /* CLOCKING_DEFS_ */
