/*
 * halm/platform/lpc/lpc43xx/system_defs.h
 * Copyright (C) 2014 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
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
/*------------------EMC clock divider configuration register------------------*/
#define CFG_DIV(value)                  BIT_FIELD((value), 5)
#define CFG_DIV_MASK                    BIT_FIELD(MASK(3), 27)
#define CFG_DIV_VALUE(reg)              FIELD_VALUE((reg), CFG_DIV_MASK, 27)
/*------------------Flash Configuration registers-----------------------------*/
#define FLASHCFG_FLASHTIM_MASK          BIT_FIELD(MASK(4), 12)
#define FLASHCFG_FLASHTIM(value)        BIT_FIELD((value), 12)
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
#define MODE_DEEP_SLEEP                 0x003000AA
#define MODE_POWERDOWN                  0x0030FCBA
#define MODE_POWERDOWN_M0SUB            0x00303CBA
#define MODE_DEEP_POWERDOWN             0x0030FF7F
/*------------------Configuration Register 0----------------------------------*/
#define CREG0_EN1KHZ                    BIT(0)
#define CREG0_EN32KHZ                   BIT(1)
#define CREG0_RESET32KHZ                BIT(2)
#define CREG0_PD32KHZ                   BIT(3)
#define CREG0_USB0PHY                   BIT(5)
#define CREG0_ALARMCTRL_MASK            BIT_FIELD(MASK(2), 6)
#define CREG0_ALARMCTRL(value)          BIT_FIELD((value), 6)
#define CREG0_ALARMCTRL_VALUE(reg) \
    FIELD_VALUE((reg), CREG0_ALARMCTRL_MASK, 6)
#define CREG0_BODLVL1_MASK              BIT_FIELD(MASK(2), 8)
#define CREG0_BODLVL1(value)            BIT_FIELD((value), 8)
#define CREG0_BODLVL1_VALUE(reg) \
    FIELD_VALUE((reg), CREG0_BODLVL1_MASK, 8)
#define CREG0_BODLVL2_MASK              BIT_FIELD(MASK(2), 10)
#define CREG0_BODLVL2(value)            BIT_FIELD((value), 10)
#define CREG0_BODLVL2_VALUE(reg) \
    FIELD_VALUE((reg), CREG0_BODLVL2_MASK, 10)
#define CREG0_SAMPLECTRL_MASK           BIT_FIELD(MASK(2), 12)
#define CREG0_SAMPLECTRL(value)         BIT_FIELD((value), 12)
#define CREG0_SAMPLECTRL_VALUE(reg) \
    FIELD_VALUE((reg), CREG0_SAMPLECTRL_MASK, 12)
#define CREG0_WAKEUP0CTRL_MASK          BIT_FIELD(MASK(2), 14)
#define CREG0_WAKEUP0CTRL(value)        BIT_FIELD((value), 14)
#define CREG0_WAKEUP0CTRL_VALUE(reg) \
    FIELD_VALUE((reg), CREG0_WAKEUP0CTRL_MASK, 14)
#define CREG0_WAKEUP1CTRL_MASK          BIT_FIELD(MASK(2), 16)
#define CREG0_WAKEUP1CTRL(value)        BIT_FIELD((value), 16)
#define CREG0_WAKEUP1CTRL_VALUE(reg) \
    FIELD_VALUE((reg), CREG0_WAKEUP1CTRL_MASK, 16)
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_LPC_LPC43XX_SYSTEM_DEFS_H_ */
