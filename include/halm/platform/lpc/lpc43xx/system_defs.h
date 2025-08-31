/*
 * halm/platform/lpc/lpc43xx/system_defs.h
 * Copyright (C) 2014 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_LPC_LPC43XX_SYSTEM_DEFS_H_
#define HALM_PLATFORM_LPC_LPC43XX_SYSTEM_DEFS_H_
/*----------------------------------------------------------------------------*/
#include <xcore/bits.h>
/*------------------Power mode registers--------------------------------------*/
#define PM_PD                           BIT(0)
/*------------------Branch clock configuration registers----------------------*/
#define CFG_RUN                         BIT(0)
#define CFG_AUTO                        BIT(1)
#define CFG_WAKEUP                      BIT(2)

#define CFG_DIV(value)                  BIT_FIELD((value), 5)
#define CFG_DIV_MASK                    BIT_FIELD(MASK(3), 5)
#define CFG_DIV_VALUE(reg)              FIELD_VALUE((reg), CFG_DIV_MASK, 5)

#define CFG_DIVSTAT(value)              BIT_FIELD((value), 27)
#define CFG_DIVSTAT_MASK                BIT_FIELD(MASK(3), 27)
#define CFG_DIVSTAT_VALUE(reg)          FIELD_VALUE((reg), CFG_DIVSTAT_MASK, 27)
/*------------------Flash Configuration registers-----------------------------*/
#define FLASHCFG_INT_CONTROL_MASK       BIT_FIELD(MASK(12), 0)
#define FLASHCFG_INT_CONTROL_VALUE(reg) \
    FIELD_VALUE((reg), FLASHCFG_INT_CONTROL_MASK, 0)

#define FLASHCFG_FLASHTIM(value)        BIT_FIELD((value), 12)
#define FLASHCFG_FLASHTIM_MASK          BIT_FIELD(MASK(4), 12)
#define FLASHCFG_FLASHTIM_VALUE(reg) \
    FIELD_VALUE((reg), FLASHCFG_FLASHTIM_MASK, 12)

#define FLASHCFG_POW                    BIT(31)
/*------------------Branch clock status registers-----------------------------*/
#define STAT_RUN                        BIT(0)
#define STAT_AUTO                       BIT(1)
#define STAT_WAKEUP                     BIT(2)
#define STAT_RUN_N                      BIT(5)
/*------------------Hardware sleep event enable register----------------------*/
#define ENA_EVENT0                      BIT(0)
#define ENA_EVENT1                      BIT(1)
/*------------------Power-down modes register---------------------------------*/
#define MODE_DEEP_SLEEP                 0x003000AAUL
#define MODE_POWERDOWN                  0x0030FCBAUL
#define MODE_POWERDOWN_M0SUB            0x00303CBAUL
#define MODE_DEEP_POWERDOWN             0x003FFF7FUL
/*------------------Configuration Register 0----------------------------------*/
#define CREG0_EN1KHZ                    BIT(0)
#define CREG0_EN32KHZ                   BIT(1)
#define CREG0_RESET32KHZ                BIT(2)
#define CREG0_PD32KHZ                   BIT(3)
#define CREG0_USB0PHY                   BIT(5)

#define CREG0_ALARMCTRL(value)          BIT_FIELD((value), 6)
#define CREG0_ALARMCTRL_MASK            BIT_FIELD(MASK(2), 6)
#define CREG0_ALARMCTRL_VALUE(reg) \
    FIELD_VALUE((reg), CREG0_ALARMCTRL_MASK, 6)

#define CREG0_BODLVL1(value)            BIT_FIELD((value), 8)
#define CREG0_BODLVL1_MASK              BIT_FIELD(MASK(2), 8)
#define CREG0_BODLVL1_VALUE(reg) \
    FIELD_VALUE((reg), CREG0_BODLVL1_MASK, 8)

#define CREG0_BODLVL2(value)            BIT_FIELD((value), 10)
#define CREG0_BODLVL2_MASK              BIT_FIELD(MASK(2), 10)
#define CREG0_BODLVL2_VALUE(reg) \
    FIELD_VALUE((reg), CREG0_BODLVL2_MASK, 10)

#define CREG0_SAMPLECTRL(value)         BIT_FIELD((value), 12)
#define CREG0_SAMPLECTRL_MASK           BIT_FIELD(MASK(2), 12)
#define CREG0_SAMPLECTRL_VALUE(reg) \
    FIELD_VALUE((reg), CREG0_SAMPLECTRL_MASK, 12)

#define CREG0_WAKEUP0CTRL(value)        BIT_FIELD((value), 14)
#define CREG0_WAKEUP0CTRL_MASK          BIT_FIELD(MASK(2), 14)
#define CREG0_WAKEUP0CTRL_VALUE(reg) \
    FIELD_VALUE((reg), CREG0_WAKEUP0CTRL_MASK, 14)

#define CREG0_WAKEUP1CTRL(value)        BIT_FIELD((value), 16)
#define CREG0_WAKEUP1CTRL_MASK          BIT_FIELD(MASK(2), 16)
#define CREG0_WAKEUP1CTRL_VALUE(reg) \
    FIELD_VALUE((reg), CREG0_WAKEUP1CTRL_MASK, 16)
/*------------------Configuration Register 1----------------------------------*/
#define CREG1_USB0_PHY_PWREN_LP         BIT(9)
#define CREG1_USB1_PHY_PWREN_LP         BIT(10)
#define CREG1_RESERVED_16               BIT(16)
#define CREG1_RESERVED_17               BIT(17)
/*------------------Configuration Register 6----------------------------------*/
enum
{
  CREG6_ETHMODE_MII   = 0,
  CREG6_ETHMODE_RMII  = 4
};

#define CREG6_ETHMODE(value)            BIT_FIELD((value), 0)
#define CREG6_ETHMODE_MASK              BIT_FIELD(MASK(3), 0)
#define CREG6_ETHMODE_VALUE(reg) \
    FIELD_VALUE((reg), CREG6_ETHMODE_MASK, 0)

#define CREG6_CTOUTCTRL                 BIT(4)
#define CREG6_I2S0_TX_SCK_IN_SEL        BIT(12)
#define CREG6_I2S0_RX_SCK_IN_SEL        BIT(13)
#define CREG6_I2S1_TX_SCK_IN_SEL        BIT(14)
#define CREG6_I2S1_RX_SCK_IN_SEL        BIT(15)
#define CREG6_EMC_CLK_SEL               BIT(16)
/*------------------EMC Clock Delay register----------------------------------*/
#define EMCDELAYCLK_CLK_DELAY(value) \
    ((value) | ((value) << 4) | ((value) << 8) | ((value) << 12))
#define EMCDELAYCLK_CLK_DELAY_MAX       7
/*------------------Chip ID register------------------------------------------*/
#define CHIPID_LPC43X0_0                0x5906002BUL
#define CHIPID_LPC43X0_1                0x6906002BUL
#define CHIPID_LPC43XX_REV_A            0x7906002BUL
#define CHIPID_LPC43XX_REV_DASH         0x4906002BUL
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_LPC_LPC43XX_SYSTEM_DEFS_H_ */
