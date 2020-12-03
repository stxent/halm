/*
 * halm/platform/lpc/gen_1/can_defs.h
 * Copyright (C) 2016 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_LPC_GEN_1_CAN_DEFS_H_
#define HALM_PLATFORM_LPC_GEN_1_CAN_DEFS_H_
/*----------------------------------------------------------------------------*/
#include <xcore/bits.h>
/*------------------Mode register---------------------------------------------*/
#define MOD_RM                          BIT(0) /* Reset Mode */
#define MOD_LOM                         BIT(1) /* Listen Only Mode */
#define MOD_STM                         BIT(2) /* Self Test Mode */
#define MOD_TPM                         BIT(3) /* Transmit Priority Mode */
#define MOD_SM                          BIT(4) /* Sleep Mode */
#define MOD_RPM                         BIT(5) /* Receive Polarity Mode */
#define MOD_TM                          BIT(7) /* Test Mode */
/*------------------Command register------------------------------------------*/
#define CMR_TR                          BIT(0) /* Transmission Request */
#define CMR_AT                          BIT(1) /* Abort Transmission */
#define CMR_RRB                         BIT(2) /* Release Receive Buffer */
#define CMR_CDO                         BIT(3) /* Clear Data Overrun */
#define CMR_SRR                         BIT(4) /* Self Reception Request */
#define CMR_STB(channel)                BIT((channel) + 5)
#define CMR_STB1                        BIT(5) /* Select TX Buffer 1 */
#define CMR_STB2                        BIT(6) /* Select TX Buffer 2 */
#define CMR_STB3                        BIT(7) /* Select TX Buffer 3 */
/*------------------Global Status register------------------------------------*/
#define GSR_RBS                         BIT(0) /* Receive Buffer Status */
#define GSR_DOS                         BIT(1) /* Data Overrun Status */
#define GSR_TBS                         BIT(2) /* Transmit Buffer Status */
#define GSR_TCS                         BIT(3) /* Transmit Complete Status */
#define GSR_RS                          BIT(4) /* Receive Status */
#define GSR_TS                          BIT(5) /* Transmit Status */
#define GSR_ES                          BIT(6) /* Error Status */
#define GSR_BS                          BIT(7) /* Bus Status */

#define GSR_RXERR(value)                BIT_FIELD((value), 16)
#define GSR_RXERR_MASK                  BIT_FIELD(MASK(8), 16)
#define GSR_RXERR_VALUE(reg)            FIELD_VALUE((reg), GSR_RXERR_MASK, 16)
#define GSR_TXERR(value)                BIT_FIELD((value), 24)
#define GSR_TXERR_MASK                  BIT_FIELD(MASK(8), 24)
#define GSR_TXERR_VALUE(reg)            FIELD_VALUE((reg), GSR_TXERR_MASK, 24)
/*------------------Interrupt and Capture Register----------------------------*/
#define ICR_RI                          BIT(0) /* Receive Interrupt */
#define ICR_TI1                         BIT(1) /* Transmit Interrupt 1 */
#define ICR_EI                          BIT(2) /* Error Warning Interrupt */
#define ICR_DOI                         BIT(3) /* Data Overrun Interrupt */
#define ICR_WUI                         BIT(4) /* Wake-Up Interrupt */
#define ICR_EPI                         BIT(5) /* Error Passive Interrupt */
#define ICR_ALI                         BIT(6) /* Arbitration Lost Interrupt */
#define ICR_BEI                         BIT(7) /* Bus Error Interrupt */
#define ICR_IDI                         BIT(8) /* ID Ready Interrupt */
#define ICR_TI2                         BIT(9) /* Transmit Interrupt 2 */
#define ICR_TI3                         BIT(10) /* Transmit Interrupt 3 */

#define ICR_ERRBIT(value)               BIT_FIELD((value), 16)
#define ICR_ERRBIT_MASK                 BIT_FIELD(MASK(5), 16)
#define ICR_ERRBIT_VALUE(reg)           FIELD_VALUE((reg), ICR_ERRBIT_MASK, 16)

#define ICR_ERRDIR                      BIT(21)

#define ICR_ERRC(value)                 BIT_FIELD((value), 22)
#define ICR_ERRC_MASK                   BIT_FIELD(MASK(2), 22)
#define ICR_ERRC_VALUE(reg)             FIELD_VALUE((reg), ICR_ERRC_MASK, 22)

#define ICR_ALCBIT(value)               BIT_FIELD((value), 24)
#define ICR_ALCBIT_MASK                 BIT_FIELD(MASK(8), 24)
#define ICR_ALCBIT_VALUE(reg)           FIELD_VALUE((reg), ICR_ALCBIT_MASK, 24)

#define ICR_TI_MASK                     (ICR_TI1 | ICR_TI2 | ICR_TI3)
/*------------------Interrupt Enable Register---------------------------------*/
#define IER_RIE                         BIT(0)
#define IER_TIE1                        BIT(1)
#define IER_EIE                         BIT(2)
#define IER_DOIE                        BIT(3)
#define IER_WUIE                        BIT(4)
#define IER_EPIE                        BIT(5)
#define IER_ALIE                        BIT(6)
#define IER_BEIE                        BIT(7)
#define IER_IDIE                        BIT(8)
#define IER_TIE2                        BIT(9)
#define IER_TIE3                        BIT(10)

#define IER_TIE_MASK                    (IER_TIE1 | IER_TIE2 | IER_TIE3)
/*------------------Bus Timing Register---------------------------------------*/
#define BTR_BRP(value)                  BIT_FIELD((value), 0)
#define BTR_BRP_MASK                    BIT_FIELD(MASK(10), 0)
#define BTR_BRP_VALUE(reg)              FIELD_VALUE((reg), BTR_BRP_MASK, 0)

#define BTR_SJW(value)                  BIT_FIELD((value), 14)
#define BTR_SJW_MASK                    BIT_FIELD(MASK(2), 14)
#define BTR_SJW_VALUE(reg)              FIELD_VALUE((reg), BTR_SJW_MASK, 14)

#define BTR_TSEG1(value)                BIT_FIELD((value), 16)
#define BTR_TSEG1_MASK                  BIT_FIELD(MASK(4), 16)
#define BTR_TSEG1_VALUE(reg)            FIELD_VALUE((reg), BTR_TSEG1_MASK, 16)

#define BTR_TSEG2(value)                BIT_FIELD((value), 20)
#define BTR_TSEG2_MASK                  BIT_FIELD(MASK(3), 20)
#define BTR_TSEG2_VALUE(reg)            FIELD_VALUE((reg), BTR_TSEG2_MASK, 20)

#define BTR_SAM                         BIT(23)
/*------------------Error Warning Limit register------------------------------*/
#define EWL_COUNTER(value)              BIT_FIELD((value), 0)
#define EWL_COUNTER_MASK                BIT_FIELD(MASK(8), 0)
#define EWL_COUNTER_VALUE(reg)          FIELD_VALUE((reg), EWL_COUNTER_MASK, 0)
/*------------------Status Register-------------------------------------------*/
/* Receive Buffer Status */
#define SR_RBS                          BIT(0)
/* Data Overrun Status */
#define SR_DOS                          BIT(1)
/* Transmit Buffer Status: 0 when locked, 1 when released */
#define SR_TBS(channel)                 BIT(((channel) << 3) + 2)
#define SR_TBS_MASK                     (SR_TBS(0) | SR_TBS(1) | SR_TBS(2))
#define SR_TBS_VALUE_TO_CHANNEL(reg)    (((reg) - 2) >> 3)
/* Transmission Complete Status: 0 when incomplete, 1 when complete */
#define SR_TCS(channel)                 BIT(((channel) << 3) + 3)
/* Receive Status */
#define SR_RS                           BIT(4)
/* Transmit Status: 0 when no transmission, 1 when transmission is ongoing */
#define SR_TS(channel)                  BIT(((channel) << 3) + 5)
/* Error Status */
#define SR_ES                           BIT(6)
/* Bus Status */
#define SR_BS                           BIT(7)
/*------------------Receive Frame Status register-----------------------------*/
#define RFS_ID_INDEX(value)             BIT_FIELD((value), 0)
#define RFS_ID_INDEX_MASK               BIT_FIELD(MASK(10), 0)
#define RFS_ID_INDEX_VALUE(reg)         FIELD_VALUE((reg), RFS_ID_INDEX_MASK, 0)

#define RFS_BP                          BIT(10)

#define RFS_DLC(value)                  BIT_FIELD((value), 16)
#define RFS_DLC_MASK                    BIT_FIELD(MASK(4), 16)
#define RFS_DLC_VALUE(reg)              FIELD_VALUE((reg), RFS_DLC_MASK, 16)

#define RFS_RTR                         BIT(30)
#define RFS_FF                          BIT(31)
/*------------------Transmit Frame Information register-----------------------*/
#define TFI_PRIO(value)                 BIT_FIELD((value), 0)
#define TFI_PRIO_MASK                   BIT_FIELD(MASK(8), 0)
#define TFI_PRIO_VALUE(reg)             FIELD_VALUE((reg), TFI_PRIO_MASK, 0)

#define TFI_DLC(value)                  BIT_FIELD((value), 16)
#define TFI_DLC_MASK                    BIT_FIELD(MASK(4), 16)
#define TFI_DLC_VALUE(reg)              FIELD_VALUE((reg), TFI_DLC_MASK, 16)

#define TFI_RTR                         BIT(30)
#define TFI_FF                          BIT(31)
/*------------------Sleep Clear register--------------------------------------*/
#define SLEEPCLR_CAN1SLEEP              BIT(1)
#define SLEEPCLR_CAN2SLEEP              BIT(2)
/*------------------Wake-up Flags register------------------------------------*/
#define WAKEFLAGS_CAN1SLEEP             BIT(1)
#define WAKEFLAGS_CAN2SLEEP             BIT(2)
/*------------------Central Transmit Status Register--------------------------*/
#define TxSR_TS1                        BIT(0)
#define TxSR_TS2                        BIT(1)
#define TxSR_TBS1                       BIT(8)
#define TxSR_TBS2                       BIT(9)
#define TxSR_TCS1                       BIT(16)
#define TxSR_TCS2                       BIT(17)
/*------------------Central Receive Status Register---------------------------*/
#define RxSR_RS1                        BIT(0)
#define RxSR_RS2                        BIT(1)
#define RxSR_RB1                        BIT(8)
#define RxSR_RB2                        BIT(9)
#define RxSR_DOS1                       BIT(16)
#define RxSR_DOS2                       BIT(17)
/*------------------Central Miscellaneous Status Register---------------------*/
#define MSR_E1                          BIT(0)
#define MSR_E2                          BIT(1)
#define MSR_BS1                         BIT(8)
#define MSR_BS2                         BIT(9)
/*------------------Acceptance Filter Mode Register---------------------------*/
#define AFMR_AccOff                     BIT(0)
#define AFMR_AccBP                      BIT(1)
#define AFMR_eFCAN                      BIT(2)
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_LPC_GEN_1_CAN_DEFS_H_ */
