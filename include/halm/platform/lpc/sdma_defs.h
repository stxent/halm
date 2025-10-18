/*
 * halm/platform/lpc/sdma_defs.h
 * Copyright (C) 2025 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_LPC_SDMA_DEFS_H_
#define HALM_PLATFORM_LPC_SDMA_DEFS_H_
/*----------------------------------------------------------------------------*/
#include <xcore/bits.h>
/*------------------Control register------------------------------------------*/
#define CTRL_ENABLE                     BIT(0)
/*------------------Interrupt Status register---------------------------------*/
#define INTSTAT_ACTIVEINT               BIT(1)
#define INTSTAT_ACTIVEERRINT            BIT(2)
/*------------------Channel Configuration registers---------------------------*/
#define CFG_PERIPHREQEN                 BIT(0)
#define CFG_HWTRIGEN                    BIT(1)
#define CFG_TRIGPOL                     BIT(4)
#define CFG_TRIGTYPE                    BIT(5)
#define CFG_TRIGBURST                   BIT(6)

#define CFG_BURSTPOWER(value)           BIT_FIELD((value), 8)
#define CFG_BURSTPOWER_MASK             BIT_FIELD(MASK(4), 8)
#define CFG_BURSTPOWER_VALUE(reg) \
    FIELD_VALUE((reg), CFG_BURSTPOWER_MASK, 8)
#define CFG_BURSTPOWER_MAX              10

#define CFG_SRCBURSTWRAP                BIT(14)
#define CFG_DSTBURSTWRAP                BIT(15)

#define CFG_CHPRIORITY(value)           BIT_FIELD((value), 16)
#define CFG_CHPRIORITY_MASK             BIT_FIELD(MASK(3), 16)
#define CFG_CHPRIORITY_VALUE(reg) \
    FIELD_VALUE((reg), CFG_CHPRIORITY_MASK, 16)
#define CFG_CHPRIORITY_MAX              7
/*------------------Channel Control and Status registers----------------------*/
#define CTLSTAT_VALIDPENDING            BIT(0)
#define CTLSTAT_TRIG                    BIT(2)
/*------------------Transfer Configuration registers--------------------------*/
#define XFERCFG_CFGVALID                BIT(0)
#define XFERCFG_RELOAD                  BIT(1)
#define XFERCFG_SWTRIG                  BIT(2)
#define XFERCFG_CLRTRIG                 BIT(3)
#define XFERCFG_SETINTA                 BIT(4)
#define XFERCFG_SETINTB                 BIT(5)

#define XFERCFG_WIDTH(value)            BIT_FIELD((value), 8)
#define XFERCFG_WIDTH_MASK              BIT_FIELD(MASK(2), 8)
#define XFERCFG_WIDTH_VALUE(reg) \
    FIELD_VALUE((reg), XFERCFG_WIDTH_MASK, 8)

#define XFERCFG_SRCINC(value)           BIT_FIELD((value), 12)
#define XFERCFG_SRCINC_MASK             BIT_FIELD(MASK(2), 12)
#define XFERCFG_SRCINC_VALUE(reg) \
    FIELD_VALUE((reg), XFERCFG_SRCINC_MASK, 12)

#define XFERCFG_DSTINC(value)           BIT_FIELD((value), 14)
#define XFERCFG_DSTINC_MASK             BIT_FIELD(MASK(2), 14)
#define XFERCFG_DSTINC_VALUE(reg) \
    FIELD_VALUE((reg), XFERCFG_DSTINC_MASK, 14)

#define XFERCFG_XFERCOUNT(value)        BIT_FIELD((value), 16)
#define XFERCFG_XFERCOUNT_MASK          BIT_FIELD(MASK(10), 16)
#define XFERCFG_XFERCOUNT_VALUE(reg) \
    FIELD_VALUE((reg), XFERCFG_XFERCOUNT_MASK, 16)
/*----------------------------------------------------------------------------*/
#define ITRIG_INMUX_RESERVED            0x0F
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_LPC_SDMA_DEFS_H_ */
