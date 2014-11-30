/*
 * platform/nxp/lpc43xx/clocking_defs.h
 * Copyright (C) 2014 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef PLATFORM_NXP_LPC43XX_CLOCKING_DEFS_H_
#define PLATFORM_NXP_LPC43XX_CLOCKING_DEFS_H_
/*----------------------------------------------------------------------------*/
#include <bits.h>
/*------------------Base clock control registers------------------------------*/
#define BASE_CLK_PD                     BIT(0) /* Output stage power down */
#define BASE_CLK_AUTOBLOCK              BIT(11)
#define BASE_CLK_SEL_MASK               BIT_FIELD(MASK(5), 24)
#define BASE_CLK_SEL(channel)           BIT_FIELD((channel), 24)
#define BASE_CLK_SEL_VALUE(reg) \
    FIELD_VALUE((reg), BASE_CLK_SEL_MASK, 24)
/*------------------Crystal Oscillator Control register-----------------------*/
#define XTAL_ENABLE                     BIT(0) /* Inverted */
#define XTAL_BYPASS                     BIT(1)
#define XTAL_HF                         BIT(2)
/*------------------PLL1 Control register-------------------------------------*/
#define PLL1_CTRL_PD                    BIT(0)
#define PLL1_CTRL_BYPASS                BIT(1)
#define PLL1_CTRL_FBSEL                 BIT(6)
#define PLL1_CTRL_DIRECT                BIT(7)
#define PLL1_CTRL_PSEL_MASK             BIT_FIELD(MASK(2), 8)
#define PLL1_CTRL_PSEL(value)           BIT_FIELD((value), 8)
#define PLL1_CTRL_AUTOBLOCK             BIT(11)
#define PLL1_CTRL_NSEL_MASK             BIT_FIELD(MASK(2), 12)
#define PLL1_CTRL_NSEL(value)           BIT_FIELD((value), 12)
#define PLL1_CTRL_MSEL_MASK             BIT_FIELD(MASK(8), 16)
#define PLL1_CTRL_MSEL(value)           BIT_FIELD((value), 16)
#define PLL1_CTRL_CLKSEL_MASK           BIT_FIELD(MASK(5), 24)
#define PLL1_CTRL_CLKSEL(value)         BIT_FIELD((value), 24)
#define PLL1_CTRL_CLKSEL_VALUE(reg) \
    FIELD_VALUE((reg), PLL1_CTRL_CLKSEL_MASK, 24)
/*------------------PLL1 Status register--------------------------------------*/
#define PLL1_STAT_LOCK                  BIT(0)
/*----------------------------------------------------------------------------*/
#endif /* PLATFORM_NXP_LPC43XX_CLOCKING_DEFS_H_ */
