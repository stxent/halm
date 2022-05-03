/*
 * halm/platform/stm32/bxcan_defs.h
 * Copyright (C) 2018 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_STM32_BXCAN_DEFS_H_
#define HALM_PLATFORM_STM32_BXCAN_DEFS_H_
/*----------------------------------------------------------------------------*/
#include <xcore/bits.h>
/*------------------Master Control Register-----------------------------------*/
/* Initialization request */
#define MCR_INRQ                        BIT(0)
/* Sleep mode request */
#define MCR_SLEEP                       BIT(1)
/* Transmit FIFO priority */
#define MCR_TXFP                        BIT(2)
/* Receive FIFO locked mode */
#define MCR_RFLM                        BIT(3)
/* No automatic retransmission */
#define MCR_NART                        BIT(4)
/* Automatic wakeup mode */
#define MCR_AWUM                        BIT(5)
/* Automatic bus-off management */
#define MCR_ABOM                        BIT(6)
/* Time triggered communication mode */
#define MCR_TTCM                        BIT(7)
/* Software master reset */
#define MCR_RESET                       BIT(15)
/* Debug freeze */
#define MCR_DBF                         BIT(16)
/*------------------Master Status Register------------------------------------*/
/* Initialization acknowledge */
#define MSR_INAK                        BIT(0)
/* Sleep acknowledge */
#define MSR_SLAK                        BIT(1)
/* Error interrupt */
#define MSR_ERRI                        BIT(2)
/* Wakeup interrupt */
#define MSR_WKUI                        BIT(3)
/* Sleep acknowledge interrupt */
#define MSR_SLAKI                       BIT(4)
/* Transmit mode */
#define MSR_TXM                         BIT(8)
/* Receive mode */
#define MSR_RXM                         BIT(9)
/* Last sample point */
#define MSR_SAMP                        BIT(10)
/* Actual value on the CAN_RX pin */
#define MSR_RX                          BIT(11)
/*------------------Transmit Status Register----------------------------------*/
/* Request completed */
#define TSR_RQCP(mailbox)               BIT((mailbox) << 3)
/* Transmission OK */
#define TSR_TXOK(mailbox)               BIT(((mailbox) << 3) + 1)
/* Arbitration lost */
#define TSR_ALST(mailbox)               BIT(((mailbox) << 3) + 2)
/* Transmission error */
#define TSR_TERR(mailbox)               BIT(((mailbox) << 3) + 3)
/* Abort request */
#define TSR_ABRQ(mailbox)               BIT(((mailbox) << 3) + 7)

/* Mailbox code */
#define TSR_CODE_MASK                   BIT_FIELD(MASK(2), 24)
#define TSR_CODE_VALUE(reg)             FIELD_VALUE((reg), TSR_CODE_MASK, 24)

/* Transmit mailbox empty */
#define TSR_TME(mailbox)                BIT((mailbox) + 26)
#define TSR_TME_MASK                    BIT_FIELD(MASK(3), 26)

/* Lowest priority flag */
#define TSR_LOW(mailbox)                BIT((mailbox) + 29)
/*------------------Receive FIFO registers------------------------------------*/
/* This value indicates how many messages are pending in the FIFO */
#define RF_FMP_MASK                     BIT_FIELD(MASK(2), 0)
#define RF_FMP_VALUE(reg)               FIELD_VALUE((reg), RF_FMP_MASK, 0)

/* FIFO full */
#define RF_FULL                         BIT(3)
/* FIFO overrun */
#define RF_FOVR                         BIT(4)
/* Release output mailbox */
#define RF_RFOM                         BIT(5)
/*------------------Interrupt Enable Register----------------------------------*/
/* Transmit mailbox empty interrupt enable */
#define IER_TMEIE                       BIT(0)
/* FIFO 0 message pending interrupt enable */
#define IER_FMPIE0                      BIT(1)
/* FIFO 0 full interrupt enable */
#define IER_FFIE0                       BIT(2)
/* FIFO 0 overrun interrupt enable */
#define IER_FOVIE0                      BIT(3)
/* FIFO 1 message pending interrupt enable */
#define IER_FMPIE1                      BIT(4)
/* FIFO 1 full interrupt enable */
#define IER_FFIE1                       BIT(5)
/* FIFO 1 overrun interrupt enable */
#define IER_FOVIE1                      BIT(6)
/* Error warning interrupt enable */
#define IER_EWGIE                       BIT(8)
/* Error passive interrupt enable */
#define IER_EPVIE                       BIT(9)
/* Bus-off interrupt enable */
#define IER_BOFIE                       BIT(10)
/* Last error code interrupt enable */
#define IER_LECIE                       BIT(11)
/* Error interrupt enable */
#define IER_ERRIE                       BIT(15)
/* Wakeup interrupt enable */
#define IER_WKUIE                       BIT(16)
/* Sleep interrupt enable */
#define IER_SLKIE                       BIT(17)
/*------------------Error Status Register-------------------------------------*/
/* Error warning flag */
#define ESR_TMEIE                       BIT(0)
/* Error passive flag */
#define ESR_FMPIE0                      BIT(1)
/* Bus-off flag */
#define ESR_FFIE0                       BIT(2)

/* Last error code */
#define ESR_LEC_MASK                    BIT_FIELD(MASK(3), 4)
#define ESR_LEC_VALUE(reg)              FIELD_VALUE((reg), ESR_LEC_MASK, 4)

/* Least significant byte of the 9-bit error counter */
#define ESR_TEC_MASK                    BIT_FIELD(MASK(8), 16)
#define ESR_TEC_VALUE(reg)              FIELD_VALUE((reg), ESR_TEC_MASK, 16)

/* Receive error counter */
#define ESR_REC_MASK                    BIT_FIELD(MASK(8), 24)
#define ESR_REC_VALUE(reg)              FIELD_VALUE((reg), ESR_REC_MASK, 24)
/*------------------Bit Timing Register---------------------------------------*/
#define BTR_BRP(value)                  BIT_FIELD((value), 0)
#define BTR_BRP_MASK                    BIT_FIELD(MASK(10), 0)
#define BTR_BRP_VALUE(reg)              FIELD_VALUE((reg), BTR_BRP_MASK, 0)

#define BTR_TS1(value)                  BIT_FIELD((value), 16)
#define BTR_TS1_MASK                    BIT_FIELD(MASK(4), 16)
#define BTR_TS1_VALUE(reg)              FIELD_VALUE((reg), BTR_TS1_MASK, 16)
#define BTR_TS1_MAX                     15

#define BTR_TS2(value)                  BIT_FIELD((value), 20)
#define BTR_TS2_MASK                    BIT_FIELD(MASK(3), 20)
#define BTR_TS2_VALUE(reg)              FIELD_VALUE((reg), BTR_TS2_MASK, 20)
#define BTR_TS2_MAX                     7

#define BTR_SJW(value)                  BIT_FIELD((value), 24)
#define BTR_SJW_MASK                    BIT_FIELD(MASK(2), 24)
#define BTR_SJW_VALUE(reg)              FIELD_VALUE((reg), BTR_SJW_MASK, 24)

/* Loop back mode */
#define BTR_LBKM                        BIT(30)
/* Silent mode */
#define BTR_SILM                        BIT(31)
/*------------------TX mailbox Identifier register----------------------------*/
/* Transmit mailbox request */
#define TI_TXRQ                         BIT(0)
/* Remote transmission request */
#define TI_RTR                          BIT(1)
/* Identifier extension */
#define TI_IDE                          BIT(2)

#define TI_EXID(value)                  BIT_FIELD((value), 3)
#define TI_EXID_MASK                    BIT_FIELD(MASK(29), 3)
#define TI_EXID_VALUE(reg)              FIELD_VALUE((reg), TI_EXID_MASK, 3)

#define TI_STID(value)                  BIT_FIELD((value), 21)
#define TI_STID_MASK                    BIT_FIELD(MASK(11), 21)
#define TI_STID_VALUE(reg)              FIELD_VALUE((reg), TI_STID_MASK, 21)
/*------------------TX mailbox DLC control and time stamp register------------*/
/* Data length code */
#define TDT_DLC(value)                  BIT_FIELD((value), 0)
#define TDT_DLC_MASK                    BIT_FIELD(MASK(4), 0)
#define TDT_DLC_VALUE(reg)              FIELD_VALUE((reg), TDT_DLC_MASK, 0)

/* Transmit global time */
#define TDT_TGT                         BIT(8)

/* Message time stamp */
#define TDT_TIME(value)                 BIT_FIELD((value), 16)
#define TDT_TIME_MASK                   BIT_FIELD(MASK(16), 16)
#define TDT_TIME_VALUE(reg)             FIELD_VALUE((reg), TDT_TIME_MASK, 16)
/*------------------Receive FIFO mailbox Identifier register------------------*/
/* Remote transmission request */
#define RI_RTR                          BIT(1)
/* Identifier extension */
#define RI_IDE                          BIT(2)

#define RI_EXID(value)                  BIT_FIELD((value), 3)
#define RI_EXID_MASK                    BIT_FIELD(MASK(29), 3)
#define RI_EXID_VALUE(reg)              FIELD_VALUE((reg), RI_EXID_MASK, 3)

#define RI_STID(value)                  BIT_FIELD((value), 21)
#define RI_STID_MASK                    BIT_FIELD(MASK(11), 21)
#define RI_STID_VALUE(reg)              FIELD_VALUE((reg), RI_STID_MASK, 21)
/*------------------RX mailbox DLC control and time stamp register------------*/
/* Data length code */
#define RDT_DLC(value)                  BIT_FIELD((value), 0)
#define RDT_DLC_MASK                    BIT_FIELD(MASK(4), 0)
#define RDT_DLC_VALUE(reg)              FIELD_VALUE((reg), RDT_DLC_MASK, 0)

/* Filter match index */
#define RDT_FMI(value)                  BIT_FIELD((value), 8)
#define RDT_FMI_MASK                    BIT_FIELD(MASK(8), 8)
#define RDT_FMI_VALUE(reg)              FIELD_VALUE((reg), RDT_FMI_MASK, 8)

/* Message time stamp */
#define RDT_TIME(value)                 BIT_FIELD((value), 16)
#define RDT_TIME_MASK                   BIT_FIELD(MASK(16), 16)
#define RDT_TIME_VALUE(reg)             FIELD_VALUE((reg), RDT_TIME_MASK, 16)
/*------------------Filter Master Register------------------------------------*/
/* Filter initialization mode */
#define FMR_FINIT                       BIT(0)

/* CAN2 start bank */
#define FMR_CAN2SB(value)               BIT_FIELD((value), 8)
#define FMR_CAN2SB_MASK                 BIT_FIELD(MASK(6), 8)
#define FMR_CAN2SB_VALUE(reg)           FIELD_VALUE((reg), FMR_CAN2SB_MASK, 8)
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_STM32_BXCAN_DEFS_H_ */
