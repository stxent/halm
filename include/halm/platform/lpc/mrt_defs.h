/*
 * halm/platform/lpc/mrt_defs.h
 * Copyright (C) 2025 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_LPC_MRT_DEFS_H_
#define HALM_PLATFORM_LPC_MRT_DEFS_H_
/*----------------------------------------------------------------------------*/
#include <xcore/bits.h>
/*----------------------------------------------------------------------------*/
#define TIMER_RESOLUTION                0x7FFFFFFFUL
/*------------------Time Interval registers-----------------------------------*/
#define INTVAL_LOAD                     BIT(31)
/*------------------Control registers-----------------------------------------*/
enum
{
  MODE_REPEAT_INTERRUPT   = 0,
  MODE_ONE_SHOT_INTERRUPT = 1,
  MODE_ONE_SHOT_BUS_STALL = 2
};

#define CTRL_INTEN                      BIT(0)

#define CTRL_MODE(value)                BIT_FIELD((value), 1)
#define CTRL_MODE_MASK                  BIT_FIELD(MASK(2), 1)
#define CTRL_MODE_VALUE(reg)            FIELD_VALUE((reg), CTRL_MODE_MASK, 1)
/*------------------Status registers------------------------------------------*/
#define STAT_INTFLAG                    BIT(0)
#define STAT_RUN                        BIT(1)
/*------------------Module Configuration register-----------------------------*/
#define MODCFG_NOC_MASK                 BIT_FIELD(MASK(4), 0)
#define MODCFG_NOC_VALUE(reg)           FIELD_VALUE((reg), MODCFG_NOC_MASK, 0)

#define MODCFG_NOB_MASK                 BIT_FIELD(MASK(4), 0)
#define MODCFG_NOB_VALUE(reg)           FIELD_VALUE((reg), MODCFG_NOB_MASK, 0)

#define MODCFG_MULTITASK                BIT(31)
/*------------------Idle Channel register-------------------------------------*/
#define IDLE_CH_CHAN_MASK               BIT_FIELD(MASK(4), 4)
#define IDLE_CH_CHAN_VALUE(reg)         FIELD_VALUE((reg), IDLE_CH_CHAN_MASK, 4)
/*------------------Global Interrupt Flag register----------------------------*/
#define IRQ_FLAG_GFLAG(channel)         BIT(channel)
#define IRQ_FLAG_GFLAG_MASK             FIELD_VALUE(MASK(4), 0)
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_LPC_MRT_DEFS_H_ */
