/*
 * halm/platform/bouffalo/uart_defs.h
 * Copyright (C) 2026 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_BOUFFALO_UART_DEFS_H_
#define HALM_PLATFORM_BOUFFALO_UART_DEFS_H_
/*----------------------------------------------------------------------------*/
#include <xcore/bits.h>
/*----------------------------------------------------------------------------*/
#define SIG_COUNT                       4
#define SIG_RTS(channel)                ((channel) * SIG_COUNT + 0)
#define SIG_CTS(channel)                ((channel) * SIG_COUNT + 1)
#define SIG_TXD(channel)                ((channel) * SIG_COUNT + 2)
#define SIG_RXD(channel)                ((channel) * SIG_COUNT + 3)
#define SIG_TOTAL                       (SIG_COUNT * 2)

#define UART_FUNCTION                   7
/*------------------TX Configuration register---------------------------------*/
enum
{
  BCNTD_5_BITS  = 4,
  BCNTD_6_BITS  = 5,
  BCNTD_7_BITS  = 6,
  BCNTD_8_BITS  = 7
};

enum
{
  BCNTP_0_5_BIT   = 0,
  BCNTP_1_BIT     = 1,
  BCNTP_1_5_BITS  = 2,
  BCNTP_2_BITS    = 3
};

#define UTX_CONFIG_EN                   BIT(0)
#define UTX_CONFIG_CTSEN                BIT(1)
#define UTX_CONFIG_FRMEN                BIT(2) /* Freerun mode */
#define UTX_CONFIG_TXPREN               BIT(4) /* Parity enable */
#define UTX_CONFIG_TXPRSEL              BIT(5) /* Odd parity enable */
#define UTX_CONFIG_IRTXEN               BIT(6)
#define UTX_CONFIG_IRTXINV              BIT(7)

#define UTX_CONFIG_TXBCNTD(value)       BIT_FIELD((value), 8)
#define UTX_CONFIG_TXBCNTD_MASK         BIT_FIELD(MASK(3), 8)
#define UTX_CONFIG_TXBCNTD_VALUE(reg) \
    FIELD_VALUE((reg), UTX_CONFIG_TXBCNTD_MASK, 8)

#define UTX_CONFIG_TXBCNTP(value)       BIT_FIELD((value), 12)
#define UTX_CONFIG_TXBCNTP_MASK         BIT_FIELD(MASK(2), 12)
#define UTX_CONFIG_TXBCNTP_VALUE(reg) \
    FIELD_VALUE((reg), UTX_CONFIG_TXBCNTP_MASK, 12)

#define UTX_CONFIG_TXLEN(value)         BIT_FIELD((value), 16)
#define UTX_CONFIG_TXLEN_MASK           BIT_FIELD(MASK(16), 16)
#define UTX_CONFIG_TXLEN_VALUE(reg) \
    FIELD_VALUE((reg), UTX_CONFIG_TXLEN_MASK, 16)
/*------------------RX Configuration register---------------------------------*/
#define URX_CONFIG_EN                   BIT(0)
#define URX_CONFIG_RTSSWM               BIT(1)
#define URX_CONFIG_RTSSWV               BIT(2)
#define URX_CONFIG_ABREN                BIT(3)
#define URX_CONFIG_RXPREN               BIT(4) /* Parity enable */
#define URX_CONFIG_RXPRSEL              BIT(5) /* Odd parity enable */
#define URX_CONFIG_IRRXEN               BIT(6)
#define URX_CONFIG_IRRXINV              BIT(7)

#define UTX_CONFIG_RXBCNTD(value)       BIT_FIELD((value), 8)
#define UTX_CONFIG_RXBCNTD_MASK         BIT_FIELD(MASK(3), 8)
#define UTX_CONFIG_RXBCNTD_VALUE(reg) \
    FIELD_VALUE((reg), UTX_CONFIG_RXBCNTD_MASK, 8)

#define URX_CONFIG_DEGEN                BIT(11)

#define UTX_CONFIG_DEGCNT(value)        BIT_FIELD((value), 12)
#define UTX_CONFIG_DEGCNT_MASK          BIT_FIELD(MASK(4), 12)
#define UTX_CONFIG_DEGCNT_VALUE(reg) \
    FIELD_VALUE((reg), UTX_CONFIG_DEGCNT_MASK, 12)

#define UTX_CONFIG_RXLEN(value)         BIT_FIELD((value), 16)
#define UTX_CONFIG_RXLEN_MASK           BIT_FIELD(MASK(16), 16)
#define UTX_CONFIG_RXLEN_VALUE(reg) \
    FIELD_VALUE((reg), UTX_CONFIG_RXLEN_MASK, 16)
/*------------------Period control register-----------------------------------*/
#define BIT_PRD_TBITPRD(value)          BIT_FIELD((value), 0)
#define BIT_PRD_TBITPRD_MASK            BIT_FIELD(MASK(16), 0)
#define BIT_PRD_TBITPRD_VALUE(reg) \
    FIELD_VALUE((reg), BIT_PRD_TBITPRD_MASK, 0)

#define BIT_PRD_RBITPRD(value)          BIT_FIELD((value), 16)
#define BIT_PRD_RBITPRD_MASK            BIT_FIELD(MASK(16), 16)
#define BIT_PRD_RBITPRD_VALUE(reg) \
    FIELD_VALUE((reg), BIT_PRD_RBITPRD_MASK, 16)

#define BIT_PRD_BITPRD_MAX              65535
/*------------------Data Configuration register-------------------------------*/
#define DATA_CONFIG_BITINV              BIT(0)
/*------------------TX IR Position control register---------------------------*/
#define UTX_IR_POSITION_TXIRPS(value)   BIT_FIELD((value), 0)
#define UTX_IR_POSITION_TXIRPS_MASK     BIT_FIELD(MASK(16), 0)
#define UTX_IR_POSITION_TXIRPS_VALUE(reg) \
    FIELD_VALUE((reg), UTX_IR_POSITION_TXIRPS_MASK, 0)

#define UTX_IR_POSITION_TXIRPP(value)   BIT_FIELD((value), 16)
#define UTX_IR_POSITION_TXIRPP_MASK     BIT_FIELD(MASK(16), 16)
#define UTX_IR_POSITION_TXIRPP_VALUE(reg) \
    FIELD_VALUE((reg), UTX_IR_POSITION_TXIRPR_MASK, 16)
/*------------------RX IR Position control register---------------------------*/
#define URX_IR_POSITION_RXIRPS(value)   BIT_FIELD((value), 0)
#define URX_IR_POSITION_RXIRPS_MASK     BIT_FIELD(MASK(16), 0)
#define URX_IR_POSITION_RXIRPS_VALUE(reg) \
    FIELD_VALUE((reg), UTX_IR_POSITION_RXIRPS_MASK, 0)
/*------------------RTO interrupt control register----------------------------*/
#define URX_RTO_TIMER_RXRTOVA(value)    BIT_FIELD((value), 0)
#define URX_RTO_TIMER_RXRTOVA_MASK      BIT_FIELD(MASK(8), 0)
#define URX_RTO_TIMER_RXRTOVA_VALUE(reg) \
    FIELD_VALUE((reg), URX_RTO_TIMER_RXRTOVA_MASK, 0)
#define URX_RTO_TIMER_RXRTOVA_MAX       255
/*------------------Interrupts Status register--------------------------------*/
#define INT_STS_TEIN              BIT(0) /* utx_end_int */
#define INT_STS_REIN              BIT(1) /* urx_end_int */
#define INT_STS_TFIN              BIT(2) /* utx_fifo_int */
#define INT_STS_RFIN              BIT(3) /* urx_fifo_int */
#define INT_STS_RRTOIN            BIT(4) /* urx_rto_int */
#define INT_STS_RPCEIN            BIT(5) /* urx_pce_int */
#define INT_STS_TFERIN            BIT(6) /* utx_fer_int */
#define INT_STS_RFERIN            BIT(7) /* urx_fer_int */
/*------------------Interrupts Mask register----------------------------------*/
#define INT_MASK_TEMS              BIT(0) /* utx_end_int */
#define INT_MASK_REMS              BIT(1) /* urx_end_int */
#define INT_MASK_TFMS              BIT(2) /* utx_fifo_int */
#define INT_MASK_RFMS              BIT(3) /* urx_fifo_int */
#define INT_MASK_RRTOMASK          BIT(4) /* urx_rto_int */
#define INT_MASK_RPCEMASK          BIT(5) /* urx_pce_int */
#define INT_MASK_TFERMASK          BIT(6) /* utx_fer_int */
#define INT_MASK_RFERMASK          BIT(7) /* urx_fer_int */
#define INT_MASK_ALL               MASK(8)
/*------------------Interrupts Clear register---------------------------------*/
#define INT_CLEAR_TECL             BIT(0) /* utx_end_int */
#define INT_CLEAR_RECL             BIT(1) /* urx_end_int */
#define INT_CLEAR_RRTOCLR          BIT(4) /* urx_rto_int */
#define INT_CLEAR_RPCECLR          BIT(5) /* urx_pce_int */
/*------------------Interrupts Enable register--------------------------------*/
#define INT_EN_TEND                BIT(0) /* utx_end_int */
#define INT_EN_REND                BIT(1) /* urx_end_int */
#define INT_EN_TFIF                BIT(2) /* utx_fifo_int */
#define INT_EN_RFIF                BIT(3) /* urx_fifo_int */
#define INT_EN_RRTO                BIT(4) /* urx_rto_int */
#define INT_EN_RPCE                BIT(5) /* urx_pce_int */
#define INT_EN_TFER                BIT(6) /* utx_fer_int */
#define INT_EN_RFER                BIT(7) /* urx_fer_int */
/*------------------Status register-------------------------------------------*/
#define STATUS_TBB                 BIT(0)
#define STATUS_RBB                 BIT(1)
/*------------------Abuto Baud detection control register---------------------*/
#define STS_URX_ABR_PRD_ABRPRDS(value) \
    BIT_FIELD((value), 0)
#define STS_URX_ABR_PRD_ABRPRDS_MASK \
    BIT_FIELD(MASK(16), 0)
#define STS_URX_ABR_PRD_ABRPRDS_VALUE(reg) \
    FIELD_VALUE((reg), STS_URX_ABR_PRD_ABRPRDS_MASK, 0)

#define STS_URX_ABR_PRD_ABRPRD(value) \
    BIT_FIELD((value), 16)
#define STS_URX_ABR_PRD_ABRPRD_MASK \
    BIT_FIELD(MASK(16), 16)
#define STS_URX_ABR_PRD_ABRPRD_VALUE(reg) \
    FIELD_VALUE((reg), STS_URX_ABR_PRD_ABRPRD_MASK, 16)
/*------------------FIFO Configuration 0 register-----------------------------*/
#define FIFO_CONFIG0_UDTEN              BIT(0)
#define FIFO_CONFIG0_UDREN              BIT(1)
#define FIFO_CONFIG0_TFICLR             BIT(2)
#define FIFO_CONFIG0_RFICLR             BIT(3)
#define FIFO_CONFIG0_TFIO               BIT(4)
#define FIFO_CONFIG0_TFIU               BIT(5)
#define FIFO_CONFIG0_RFIO               BIT(6)
#define FIFO_CONFIG0_RFIU               BIT(7)
/*------------------FIFO Configuration 1 register-----------------------------*/
#define FIFO_CONFIG1_TFICNT(value)      BIT_FIELD((value), 0)
#define FIFO_CONFIG1_TFICNT_MASK        BIT_FIELD(MASK(6), 0)
#define FIFO_CONFIG1_TFICNT_VALUE(reg) \
    FIELD_VALUE((reg), FIFO_CONFIG1_TFICNT_MASK, 0)

#define FIFO_CONFIG1_RFICNT(value)      BIT_FIELD((value), 8)
#define FIFO_CONFIG1_RFICNT_MASK        BIT_FIELD(MASK(6), 8)
#define FIFO_CONFIG1_RFICNT_VALUE(reg) \
    FIELD_VALUE((reg), FIFO_CONFIG1_RFICNT_MASK, 8)

#define FIFO_CONFIG1_TFITH(value)       BIT_FIELD((value), 16)
#define FIFO_CONFIG1_TFITH_MASK         BIT_FIELD(MASK(5), 16)
#define FIFO_CONFIG1_TFITH_VALUE(reg) \
    FIELD_VALUE((reg), FIFO_CONFIG1_TFITH_MASK, 16)

#define FIFO_CONFIG1_RFITH(value)       BIT_FIELD((value), 24)
#define FIFO_CONFIG1_RFITH_MASK         BIT_FIELD(MASK(5), 24)
#define FIFO_CONFIG1_RFITH_VALUE(reg) \
    FIELD_VALUE((reg), FIFO_CONFIG1_RFITH_MASK, 24)

#define FIFO_CONFIG1_FITH_MAX           31
/*------------------UART Signal Select register-------------------------------*/
#define UART_SIG_SEL_SIG(number, value) BIT_FIELD((value), (number) * 4)
#define UART_SIG_SEL_SIG_MASK(number)   BIT_FIELD(MASK(4), (number) * 4)
#define UART_SIG_SEL_SIG_VALUE(number, reg) \
    FIELD_VALUE((reg), UART_SIG_SEL_SIG_MASK(number), (number) * 4)
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_BOUFFALO_UART_DEFS_H_ */
