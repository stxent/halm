/*
 * platform/nxp/lpc17xx/clocking_defs.h
 * Copyright (C) 2013 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef PLATFORM_NXP_LPC17XX_CLOCKING_DEFS_H_
#define PLATFORM_NXP_LPC17XX_CLOCKING_DEFS_H_
/*----------------------------------------------------------------------------*/
#include <bits.h>
/*------------------System Controls and Status register-----------------------*/
#define SCS_FREQRANGE                   BIT(4) /* Set for 15 - 25 MHz range */
#define SCS_OSCEN                       BIT(5)
#define SCS_OSCSTAT                     BIT(6)
/*------------------Clock Source Select register------------------------------*/
#define CLKSRCSEL_MASK                  BIT_FIELD(MASK(2), 0)
#define CLKSRCSEL_IRC                   BIT_FIELD(0, 0)
#define CLKSRCSEL_MAIN                  BIT_FIELD(1, 0)
#define CLKSRCSEL_RTC                   BIT_FIELD(2, 0)
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
#define PLL1STAT_LOCK                   BIT(10)
/*------------------PLL Feed registers----------------------------------------*/
#define PLLFEED_FIRST                   0xAA
#define PLLFEED_SECOND                  0x55
/*------------------Flash Accelerator Configuration register------------------*/
#define FLASHCFG_TIME_MASK              BIT_FIELD(MASK(4), 12)
#define FLASHCFG_TIME(value)            BIT_FIELD((value), 12)
/*----------------------------------------------------------------------------*/
#endif /* PLATFORM_NXP_LPC17XX_CLOCKING_DEFS_H_ */
