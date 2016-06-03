/*
 * halm/platform/nxp/lpc11exx/clocking_defs.h
 * Copyright (C) 2014 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef HALM_PLATFORM_NXP_LPC11EXX_CLOCKING_DEFS_H_
#define HALM_PLATFORM_NXP_LPC11EXX_CLOCKING_DEFS_H_
/*----------------------------------------------------------------------------*/
#include <xcore/bits.h>
/*------------------Clock Source Update registers-----------------------------*/
#define CLKUEN_ENA                      BIT(0)
/*------------------Clock Output Source Select register-----------------------*/
#define CLKOUTCLKSEL_IRC                BIT_FIELD(0, 0)
#define CLKOUTCLKSEL_SYSOSC             BIT_FIELD(1, 0)
#define CLKOUTCLKSEL_WDT                BIT_FIELD(2, 0)
#define CLKOUTCLKSEL_MAIN_CLOCK         BIT_FIELD(3, 0)
/*------------------Main Clock Source Select register-------------------------*/
#define MAINCLKSEL_IRC                  BIT_FIELD(0, 0)
#define MAINCLKSEL_PLL_INPUT            BIT_FIELD(1, 0)
#define MAINCLKSEL_WDT                  BIT_FIELD(2, 0)
#define MAINCLKSEL_PLL_OUTPUT           BIT_FIELD(3, 0)
/*------------------PLL Clock Source Select registers-------------------------*/
#define PLLCLKSEL_IRC                   BIT_FIELD(0, 0)
#define PLLCLKSEL_SYSOSC                BIT_FIELD(1, 0)
/*------------------PLL Control registers-------------------------------------*/
#define PLLCTRL_MSEL_MASK               BIT_FIELD(MASK(5), 0)
#define PLLCTRL_MSEL(value)             BIT_FIELD((value), 0)
#define PLLCTRL_PSEL_MASK               BIT_FIELD(MASK(2), 5)
#define PLLCTRL_PSEL(value)             BIT_FIELD((value), 5)
/*------------------PLL Status registers--------------------------------------*/
#define PLLSTAT_LOCK                    BIT(0)
/*------------------System Oscillator Control register------------------------*/
#define SYSOSCCTRL_BYPASS               BIT(0)
#define SYSOSCCTRL_FREQRANGE            BIT(1) /* Set for 15 - 25 MHz range */
/*------------------Watchdog Oscillator Control register----------------------*/
#define WDTOSCCTRL_DIVSEL_MASK          BIT_FIELD(MASK(5), 0)
#define WDTOSCCTRL_DIVSEL(value)        BIT_FIELD((value), 0)
#define WDTOSCCTRL_DIVSEL_VALUE(reg) \
    FIELD_VALUE((reg), WDTOSCCTRL_DIVSEL_MASK, 0)
#define WDTOSCCTRL_FREQSEL_MASK         BIT_FIELD(MASK(4), 5)
#define WDTOSCCTRL_FREQSEL(value)       BIT_FIELD((value), 5)
#define WDTOSCCTRL_FREQSEL_VALUE(reg) \
    FIELD_VALUE((reg), WDTOSCCTRL_FREQSEL_MASK, 5)
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_NXP_LPC11EXX_CLOCKING_DEFS_H_ */
