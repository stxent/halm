/*
 * halm/platform/lpc/lpc17xx/clocking_defs.h
 * Copyright (C) 2013 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef HALM_PLATFORM_LPC_LPC17XX_CLOCKING_DEFS_H_
#define HALM_PLATFORM_LPC_LPC17XX_CLOCKING_DEFS_H_
/*----------------------------------------------------------------------------*/
#include <xcore/bits.h>
/*------------------System Controls and Status register-----------------------*/
#define SCS_FREQRANGE                   BIT(4) /* Set for 15 - 25 MHz range */
#define SCS_OSCEN                       BIT(5)
#define SCS_OSCSTAT                     BIT(6)
/*------------------Clock Source Select register------------------------------*/
enum
{
  CLKSRCSEL_IRC  = 0,
  CLKSRCSEL_MAIN = 1,
  CLKSRCSEL_RTC  = 2
};
/*------------------Clock Output Configuration register-----------------------*/
enum
{
  CLKOUTCFG_CPU  = 0,
  CLKOUTCFG_MAIN = 1,
  CLKOUTCFG_IRC  = 2,
  CLKOUTCFG_USB  = 3,
  CLKOUTCFG_RTC  = 4
};

#define CLKOUTCFG_SEL_MASK              BIT_FIELD(MASK(4), 0)
#define CLKOUTCFG_SEL(value)            BIT_FIELD((value), 0)
#define CLKOUTCFG_SEL_VALUE(reg) \
    FIELD_VALUE((reg), CLKOUTCFG_SEL_MASK, 0)
#define CLKOUTCFG_DIV_MASK              BIT_FIELD(MASK(4), 4)
#define CLKOUTCFG_DIV(value)            BIT_FIELD((value), 4)
#define CLKOUTCFG_DIV_VALUE(reg) \
    FIELD_VALUE((reg), CLKOUTCFG_DIV_MASK, 4)
#define CLKOUTCFG_EN                    BIT(8)
#define CLKOUTCFG_ACT                   BIT(9)
/*------------------PLL0 Control register-------------------------------------*/
#define PLL0CON_ENABLE                  BIT(0)
#define PLL0CON_CONNECT                 BIT(1)
/*------------------PLL0 Configuration register-------------------------------*/
#define PLL0CFG_MSEL_MASK               BIT_FIELD(MASK(15), 0)
#define PLL0CFG_MSEL(value)             BIT_FIELD((value), 0)
#define PLL0CFG_NSEL_MASK               BIT_FIELD(MASK(8), 16)
#define PLL0CFG_NSEL(value)             BIT_FIELD((value), 16)
/*------------------PLL0 Status register--------------------------------------*/
/* Read-back for the Enable bit */
#define PLL0STAT_ENABLED                BIT(24)
/* Read-back for the Connect bit */
#define PLL0STAT_CONNECTED              BIT(25)
#define PLL0STAT_LOCK                   BIT(26)
/*------------------PLL1 Control register-------------------------------------*/
#define PLL1CON_ENABLE                  BIT(0)
#define PLL1CON_CONNECT                 BIT(1)
/*------------------PLL1 Configuration register-------------------------------*/
#define PLL1CFG_MSEL_MASK               BIT_FIELD(MASK(5), 0)
#define PLL1CFG_MSEL(value)             BIT_FIELD((value), 0)
#define PLL1CFG_PSEL_MASK               BIT_FIELD(MASK(2), 5)
#define PLL1CFG_PSEL(value)             BIT_FIELD((value), 5)
/*------------------PLL1 Status register--------------------------------------*/
/* Read-back for the Enable bit */
#define PLL1STAT_ENABLED                BIT(8)
/* Read-back for the Connect bit */
#define PLL1STAT_CONNECTED              BIT(9)
#define PLL1STAT_LOCK                   BIT(10)
/*------------------PLL Feed registers----------------------------------------*/
#define PLLFEED_FIRST                   0xAA
#define PLLFEED_SECOND                  0x55
/*------------------USB Clock Configuration register--------------------------*/
#define USBCLKCFG_USBSEL_MASK           BIT_FIELD(MASK(4), 0)
#define USBCLKCFG_USBSEL(value)         BIT_FIELD((value), 0)
#define USBCLKCFG_USBSEL_VALUE(reg) \
    FIELD_VALUE((reg), USBCLKCFG_USBSEL_MASK, 0)
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_LPC_LPC17XX_CLOCKING_DEFS_H_ */
