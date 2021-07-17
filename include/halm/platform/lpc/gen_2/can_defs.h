/*
 * halm/platform/lpc/gen_2/can_defs.h
 * Copyright (C) 2017 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_LPC_GEN_2_CAN_DEFS_H_
#define HALM_PLATFORM_LPC_GEN_2_CAN_DEFS_H_
/*----------------------------------------------------------------------------*/
#include <xcore/bits.h>
/*------------------Control register------------------------------------------*/
/* Initialization mode enable */
#define CNTL_INIT                       BIT(0)
/* Module interrupt enable */
#define CNTL_IE                         BIT(1)
/* Status change interrupt enable */
#define CNTL_SIE                        BIT(2)
/* Error interrupt enable */
#define CNTL_EIE                        BIT(3)
/* Disable automatic retransmission */
#define CNTL_DAR                        BIT(5)
/* Configuration change enable */
#define CNTL_CCE                        BIT(6)
/* Test mode enable */
#define CNTL_TEST                       BIT(7)
/*------------------Status register-------------------------------------------*/
enum
{
  LEC_NO_ERROR    = 0x00,
  LEC_STUFF_ERROR = 0x01,
  LEC_FORM_ERROR  = 0x02,
  LEC_ACK_ERROR   = 0x03,
  LEC_BIT_1_ERROR = 0x04,
  LEC_BIT_0_ERROR = 0x05,
  LEC_CRC_ERROR   = 0x06,
  LEC_UNUSED      = 0x07
};

#define STAT_LEC(value)                 BIT_FIELD((value), 0)
#define STAT_LEC_MASK                   BIT_FIELD(MASK(3), 0)
#define STAT_LEC_VALUE(reg)             FIELD_VALUE((reg), STAT_LEC_MASK, 0)

#define STAT_TXOK                       BIT(3)
#define STAT_RXOK                       BIT(4)
#define STAT_EPASS                      BIT(5)
#define STAT_EWARN                      BIT(6)
#define STAT_BOFF                       BIT(7)
/*------------------Error Counter---------------------------------------------*/
#define EC_TEC_MASK                     BIT_FIELD(MASK(8), 0)
#define EC_TEC_VALUE(reg)               FIELD_VALUE((reg), EC_TEC_MASK, 0)

#define EC_REC_MASK                     BIT_FIELD(MASK(7), 8)
#define EC_REC_VALUE(reg)               FIELD_VALUE((reg), EC_REC_MASK, 8)

#define EC_RP                           BIT(15)
/*------------------Bit Timing register---------------------------------------*/
#define BT_BRP(value)                   BIT_FIELD((value), 0)
#define BT_BRP_MASK                     BIT_FIELD(MASK(6), 0)
#define BT_BRP_VALUE(reg)               FIELD_VALUE((reg), BT_BRP_MASK, 0)

#define BT_SJW(value)                   BIT_FIELD((value), 6)
#define BT_SJW_MASK                     BIT_FIELD(MASK(2), 6)
#define BT_SJW_VALUE(reg)               FIELD_VALUE((reg), BT_SJW_MASK, 6)

#define BT_TSEG1(value)                 BIT_FIELD((value), 8)
#define BT_TSEG1_MASK                   BIT_FIELD(MASK(4), 8)
#define BT_TSEG1_VALUE(reg)             FIELD_VALUE((reg), BT_TSEG1_MASK, 8)

#define BT_TSEG2(value)                 BIT_FIELD((value), 12)
#define BT_TSEG2_MASK                   BIT_FIELD(MASK(3), 12)
#define BT_TSEG2_VALUE(reg)             FIELD_VALUE((reg), BT_TSEG2_MASK, 12)
/*------------------Interrupt register----------------------------------------*/
enum
{
  INT_NONE   = 0x0000,
  INT_STATUS = 0x8000
};

#define INT_ID_MASK                     BIT_FIELD(MASK(16), 0)
#define INT_ID_VALUE(reg)               FIELD_VALUE((reg), INT_ID_MASK, 0)
/*------------------Test register---------------------------------------------*/
#define TEST_BASIC                      BIT(2)
#define TEST_SILENT                     BIT(3)
#define TEST_LBACK                      BIT(4)
#define TEST_TX1_0(value)               BIT_FIELD((value), 5)
#define TEST_RX                         BIT(7)
/*------------------Command Request registers---------------------------------*/
#define CMDREQ_NUMBER(value)            BIT_FIELD((value), 0)
#define CMDREQ_NUMBER_MASK              BIT_FIELD(MASK(6), 0)
#define CMDREQ_NUMBER_VALUE(reg) \
    FIELD_VALUE((reg), CMDREQ_NUMBER_MASK, 0)

#define CMDREQ_BUSY                     BIT(15)
/*------------------Command Mask registers (read and write directions)--------*/
#define CMDMSK_DATA_B                   BIT(0)
#define CMDMSK_DATA_A                   BIT(1)

#define CMDMSK_TXRQST                   BIT(2)
#define CMDMSK_NEWDAT                   BIT(2)

#define CMDMSK_CLRINTPND                BIT(3)
#define CMDMSK_CTRL                     BIT(4)
#define CMDMSK_ARB                      BIT(5)
#define CMDMSK_MASK                     BIT(6)
#define CMDMSK_WR                       BIT(7)
/*------------------Command Mask registers------------------------------------*/
#define MSK1_EXT_ID(value)              BIT_FIELD((value), 0)
#define MSK1_EXT_ID_MASK                BIT_FIELD(MASK(16), 0)
#define MSK1_EXT_ID_VALUE(reg)          FIELD_VALUE((reg), MSK1_EXT_ID_MASK, 0)

#define MSK2_EXT_ID(value)              BIT_FIELD((value), 0)
#define MSK2_EXT_ID_MASK                BIT_FIELD(MASK(13), 0)
#define MSK2_EXT_ID_VALUE(reg)          FIELD_VALUE((reg), MSK2_EXT_ID_MASK, 0)

#define MSK2_STD_ID(value)              BIT_FIELD((value), 2)
#define MSK2_STD_ID_MASK                BIT_FIELD(MASK(11), 2)
#define MSK2_STD_ID_VALUE(reg)          FIELD_VALUE((reg), MSK2_STD_ID_MASK, 2)

#define MSK1_EXT_ID_FROM_MASK(value)    (MSK1_EXT_ID(value) & MSK1_EXT_ID_MASK)
#define MSK2_EXT_ID_FROM_MASK(value)    MSK2_EXT_ID((value) >> 16)
#define MSK2_STD_ID_FROM_MASK(value)    MSK2_STD_ID(value)

#define MSK2_MDIR                       BIT(14)
#define MSK2_MXTD                       BIT(15)
/*------------------Command Arbitration registers-----------------------------*/
#define ARB1_EXT_ID(value)              BIT_FIELD((value), 0)
#define ARB1_EXT_ID_MASK                BIT_FIELD(MASK(16), 0)
#define ARB1_EXT_ID_VALUE(reg)          FIELD_VALUE((reg), ARB1_EXT_ID_MASK, 0)

#define ARB2_EXT_ID(value)              BIT_FIELD((value), 0)
#define ARB2_EXT_ID_MASK                BIT_FIELD(MASK(13), 0)
#define ARB2_EXT_ID_VALUE(reg)          FIELD_VALUE((reg), ARB2_EXT_ID_MASK, 0)

#define ARB2_STD_ID(value)              BIT_FIELD((value), 2)
#define ARB2_STD_ID_MASK                BIT_FIELD(MASK(11), 2)
#define ARB2_STD_ID_VALUE(reg)          FIELD_VALUE((reg), ARB2_STD_ID_MASK, 2)

#define ARB1_EXT_ID_FROM_ID(value)      (ARB1_EXT_ID(value) & ARB1_EXT_ID_MASK)
#define ARB2_EXT_ID_FROM_ID(value)      ARB2_EXT_ID((value) >> 16)
#define ARB2_STD_ID_FROM_ID(value)      ARB2_STD_ID(value)

#define EXT_ID_FROM_ARB(arb1, arb2) \
    (ARB1_EXT_ID_VALUE(arb1) | (ARB2_EXT_ID_VALUE(arb2) << 16))

#define ARB2_DIR                        BIT(13)
#define ARB2_XTD                        BIT(14)
#define ARB2_MSGVAL                     BIT(15)
/*------------------Message Control registers---------------------------------*/
#define MCTRL_DLC(value)                BIT_FIELD((value), 0)
#define MCTRL_DLC_MASK                  BIT_FIELD(MASK(4), 0)
#define MCTRL_DLC_VALUE(reg)            FIELD_VALUE((reg), MCTRL_DLC_MASK, 0)

#define MCTRL_EOB                       BIT(7)
#define MCTRL_TXRQST                    BIT(8)
#define MCTRL_RMTEN                     BIT(9)
#define MCTRL_RXIE                      BIT(10)
#define MCTRL_TXIE                      BIT(11)
#define MCTRL_UMASK                     BIT(12)
#define MCTRL_INTPND                    BIT(13)
#define MCTRL_MSGLST                    BIT(14)
#define MCTRL_NEWDAT                    BIT(15)
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_LPC_GEN_2_CAN_DEFS_H_ */
