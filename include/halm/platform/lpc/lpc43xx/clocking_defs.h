/*
 * halm/platform/lpc/lpc43xx/clocking_defs.h
 * Copyright (C) 2014 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef HALM_PLATFORM_LPC_LPC43XX_CLOCKING_DEFS_H_
#define HALM_PLATFORM_LPC_LPC43XX_CLOCKING_DEFS_H_
/*----------------------------------------------------------------------------*/
#include <xcore/bits.h>
/*------------------Base clock control registers------------------------------*/
#define BASE_PD                         BIT(0) /* Output stage power down */
#define BASE_AUTOBLOCK                  BIT(11)
#define BASE_CLK_SEL_MASK               BIT_FIELD(MASK(5), 24)
#define BASE_CLK_SEL(channel)           BIT_FIELD((channel), 24)
#define BASE_CLK_SEL_VALUE(reg) \
    FIELD_VALUE((reg), BASE_CLK_SEL_MASK, 24)
/*------------------Crystal Oscillator Control register-----------------------*/
#define XTAL_ENABLE                     BIT(0) /* Inverted */
#define XTAL_BYPASS                     BIT(1)
#define XTAL_HF                         BIT(2)
/*------------------Common PLL0 Control registers-----------------------------*/
#define PLL0_CTRL_BYPASS                BIT(1)
#define PLL0_CTRL_DIRECTI               BIT(2)
#define PLL0_CTRL_DIRECTO               BIT(3)
#define PLL0_CTRL_CLKEN                 BIT(4)
#define PLL0_CTRL_FRM                   BIT(6)
/*------------------Audio PLL0 Control register-------------------------------*/
#define PLL0_PLLFRACT_REQ               BIT(12)
#define PLL0_SEL_EXT                    BIT(13)
#define PLL0_MOD_PD                     BIT(14)
/*------------------PLL0 M-Divider register-----------------------------------*/
#define PLL0_MDIV_MDEC_MASK             BIT_FIELD(MASK(17), 0)
#define PLL0_MDIV_MDEC(value)           BIT_FIELD((value), 0)
#define PLL0_MDIV_SELP_MASK             BIT_FIELD(MASK(5), 17)
#define PLL0_MDIV_SELP(value)           BIT_FIELD((value), 17)
#define PLL0_MDIV_SELI_MASK             BIT_FIELD(MASK(6), 22)
#define PLL0_MDIV_SELI(value)           BIT_FIELD((value), 22)
#define PLL0_MDIV_SELR_MASK             BIT_FIELD(MASK(4), 28)
#define PLL0_MDIV_SELR(value)           BIT_FIELD((value), 28)
/*------------------PLL0 NP-Divider register----------------------------------*/
#define PLL0_NP_DIV_PDEC_MASK           BIT_FIELD(MASK(7), 0)
#define PLL0_NP_DIV_PDEC(value)         BIT_FIELD((value), 0)
#define PLL0_NP_DIV_NDEC_MASK           BIT_FIELD(MASK(8), 12)
#define PLL0_NP_DIV_NDEC(value)         BIT_FIELD((value), 12)
/*------------------PLL0 Status register--------------------------------------*/
#define PLL0_STAT_LOCK                  BIT(0)
#define PLL0_STAT_FR                    BIT(1)
/*------------------PLL1 Control register-------------------------------------*/
#define PLL1_CTRL_BYPASS                BIT(1)
#define PLL1_CTRL_FBSEL                 BIT(6)
#define PLL1_CTRL_DIRECT                BIT(7)
#define PLL1_CTRL_PSEL_MASK             BIT_FIELD(MASK(2), 8)
#define PLL1_CTRL_PSEL(value)           BIT_FIELD((value), 8)
#define PLL1_CTRL_NSEL_MASK             BIT_FIELD(MASK(2), 12)
#define PLL1_CTRL_NSEL(value)           BIT_FIELD((value), 12)
#define PLL1_CTRL_MSEL_MASK             BIT_FIELD(MASK(8), 16)
#define PLL1_CTRL_MSEL(value)           BIT_FIELD((value), 16)
/*------------------PLL1 Status register--------------------------------------*/
#define PLL1_STAT_LOCK                  BIT(0)
/*------------------Integer Divider register----------------------------------*/
#define IDIV_PD                         BIT(0)
#define IDIV_DIVIDER_MASK               BIT_FIELD(MASK(8), 2) /* From 2 to 8 */
#define IDIV_DIVIDER(value)             BIT_FIELD((value), 2)
#define IDIV_DIVIDER_VALUE(reg)         FIELD_VALUE((reg), IDIV_DIVIDER_MASK, 2)
#define IDIV_AUTOBLOCK                  BIT(11)
#define IDIV_CLK_SEL_MASK               BIT_FIELD(MASK(5), 24)
#define IDIV_CLK_SEL(channel)           BIT_FIELD((channel), 24)
#define IDIV_CLK_SEL_VALUE(reg) \
    FIELD_VALUE((reg), IDIV_CLK_SEL_MASK, 24)
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_LPC_LPC43XX_CLOCKING_DEFS_H_ */
