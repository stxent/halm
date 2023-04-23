/*
 * halm/platform/numicro/can_defs.h
 * Copyright (C) 2023 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_NUMICRO_CAN_DEFS_H_
#define HALM_PLATFORM_NUMICRO_CAN_DEFS_H_
/*----------------------------------------------------------------------------*/
#include <xcore/bits.h>
/*------------------Control register------------------------------------------*/
/* Initialization mode enable */
#define CON_INIT                        BIT(0)
/* Module interrupt enable */
#define CON_IE                          BIT(1)
/* Status change interrupt enable */
#define CON_SIE                         BIT(2)
/* Error interrupt enable */
#define CON_EIE                         BIT(3)
/* Disable automatic retransmission */
#define CON_DAR                         BIT(5)
/* Configuration change enable */
#define CON_CCE                         BIT(6)
/* Test mode enable */
#define CON_TEST                        BIT(7)
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

#define STATUS_LEC(value)               BIT_FIELD((value), 0)
#define STATUS_LEC_MASK                 BIT_FIELD(MASK(3), 0)
#define STATUS_LEC_VALUE(reg)           FIELD_VALUE((reg), STATUS_LEC_MASK, 0)

#define STATUS_TXOK                     BIT(3)
#define STATUS_RXOK                     BIT(4)
#define STATUS_EPASS                    BIT(5)
#define STATUS_EWARN                    BIT(6)
#define STATUS_BOFF                     BIT(7)
/*------------------Error Counter---------------------------------------------*/
#define ERR_TEC_MASK                    BIT_FIELD(MASK(8), 0)
#define ERR_TEC_VALUE(reg)              FIELD_VALUE((reg), ERR_TEC_MASK, 0)

#define ERR_REC_MASK                    BIT_FIELD(MASK(7), 8)
#define ERR_REC_VALUE(reg)              FIELD_VALUE((reg), ERR_REC_MASK, 8)

#define ERR_RP                          BIT(15)
/*------------------Bit Timing register---------------------------------------*/
#define BRP_BRPE_MAX                    1024

#define BTIME_BRP(value)                BIT_FIELD((value), 0)
#define BTIME_BRP_MASK                  BIT_FIELD(MASK(6), 0)
#define BTIME_BRP_VALUE(reg)            FIELD_VALUE((reg), BTIME_BRP_MASK, 0)
#define BTIME_BRP_WIDTH                 6

#define BTIME_SJW(value)                BIT_FIELD((value), 6)
#define BTIME_SJW_MASK                  BIT_FIELD(MASK(2), 6)
#define BTIME_SJW_VALUE(reg)            FIELD_VALUE((reg), BTIME_SJW_MASK, 6)

#define BTIME_TSEG1(value)              BIT_FIELD((value), 8)
#define BTIME_TSEG1_MASK                BIT_FIELD(MASK(4), 8)
#define BTIME_TSEG1_VALUE(reg)          FIELD_VALUE((reg), BTIME_TSEG1_MASK, 8)
#define BTIME_TSEG1_MAX                 15
#define BTIME_TSEG1_MIN                 1

#define BTIME_TSEG2(value)              BIT_FIELD((value), 12)
#define BTIME_TSEG2_MASK                BIT_FIELD(MASK(3), 12)
#define BTIME_TSEG2_VALUE(reg)          FIELD_VALUE((reg), BTIME_TSEG2_MASK, 12)
#define BTIME_TSEG2_MAX                 7
/*------------------Interrupt register----------------------------------------*/
enum
{
  INTID_NONE   = 0x0000,
  INTID_STATUS = 0x8000
};

#define IIDR_INTID_MASK                 BIT_FIELD(MASK(16), 0)
#define IIDR_INTID_VALUE(reg)           FIELD_VALUE((reg), IIDR_INTID_MASK, 0)
/*------------------Test register---------------------------------------------*/
#define TEST_BASIC                      BIT(2)
#define TEST_SILENT                     BIT(3)
#define TEST_LBACK                      BIT(4)
#define TEST_TX1_0(value)               BIT_FIELD((value), 5)
#define TEST_RX                         BIT(7)
/*------------------Command Request registers---------------------------------*/
#define CREQ_NUMBER(value)              BIT_FIELD((value), 0)
#define CREQ_NUMBER_MASK                BIT_FIELD(MASK(6), 0)
#define CREQ_NUMBER_VALUE(reg) \
    FIELD_VALUE((reg), CREQ_NUMBER_MASK, 0)

#define CREQ_BUSY                       BIT(15)
/*------------------Command Mask registers (read and write directions)--------*/
#define CMASK_DATA_B                    BIT(0)
#define CMASK_DATA_A                    BIT(1)

#define CMASK_TXRQST                    BIT(2)
#define CMASK_NEWDAT                    BIT(2)

#define CMASK_CLRINTPND                 BIT(3)
#define CMASK_CONTROL                   BIT(4)
#define CMASK_ARB                       BIT(5)
#define CMASK_MASK                      BIT(6)
#define CMASK_WR                        BIT(7)
/*------------------Command Mask registers------------------------------------*/
#define MASK1_EXT_ID(value)             BIT_FIELD((value), 0)
#define MASK1_EXT_ID_MASK               BIT_FIELD(MASK(16), 0)
#define MASK1_EXT_ID_VALUE(reg)         FIELD_VALUE((reg), MASK1_EXT_ID_MASK, 0)

#define MASK2_EXT_ID(value)             BIT_FIELD((value), 0)
#define MASK2_EXT_ID_MASK               BIT_FIELD(MASK(13), 0)
#define MASK2_EXT_ID_VALUE(reg)         FIELD_VALUE((reg), MASK2_EXT_ID_MASK, 0)

#define MASK2_STD_ID(value)             BIT_FIELD((value), 2)
#define MASK2_STD_ID_MASK               BIT_FIELD(MASK(11), 2)
#define MASK2_STD_ID_VALUE(reg)         FIELD_VALUE((reg), MASK2_STD_ID_MASK, 2)

#define MASK1_EXT_ID_FROM_MASK(value) \
    (MASK1_EXT_ID(value) & MASK1_EXT_ID_MASK)
#define MASK2_EXT_ID_FROM_MASK(value)   MASK2_EXT_ID((value) >> 16)
#define MASK2_STD_ID_FROM_MASK(value)   MASK2_STD_ID(value)

#define MASK2_MDIR                      BIT(14)
#define MASK2_MXTD                      BIT(15)
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
#define MCON_DLC(value)                 BIT_FIELD((value), 0)
#define MCON_DLC_MASK                   BIT_FIELD(MASK(4), 0)
#define MCON_DLC_VALUE(reg)             FIELD_VALUE((reg), MCON_DLC_MASK, 0)

#define MCON_EOB                        BIT(7)
#define MCON_TXRQST                     BIT(8)
#define MCON_RMTEN                      BIT(9)
#define MCON_RXIE                       BIT(10)
#define MCON_TXIE                       BIT(11)
#define MCON_UMASK                      BIT(12)
#define MCON_INTPND                     BIT(13)
#define MCON_MSGLST                     BIT(14)
#define MCON_NEWDAT                     BIT(15)
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_NUMICRO_CAN_DEFS_H_ */
