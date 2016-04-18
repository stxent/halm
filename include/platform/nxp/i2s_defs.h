/*
 * platform/nxp/i2s_defs.h
 * Copyright (C) 2014 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef HALM_PLATFORM_NXP_I2S_DEFS_H_
#define HALM_PLATFORM_NXP_I2S_DEFS_H_
/*----------------------------------------------------------------------------*/
#include <bits.h>
/*----------------------------------------------------------------------------*/
enum
{
  WORDWIDTH_8BIT  = 0,
  WORDWIDTH_16BIT = 1,
  WORDWIDTH_32BIT = 3
};
/*------------------Digital Audio Output register-----------------------------*/
#define DAO_WORDWIDTH_MASK              BIT_FIELD(MASK(2), 0)
#define DAO_WORDWIDTH(value)            BIT_FIELD((value), 0)
#define DAO_WORDWIDTH_VALUE(reg) \
    FIELD_VALUE((reg), DAO_WORDWIDTH_MASK, 0)
#define DAO_MONO                        BIT(2)
#define DAO_STOP                        BIT(3)
#define DAO_RESET                       BIT(4)
#define DAO_WS_SEL                      BIT(5)
#define DAO_WS_HALFPERIOD_MASK          BIT_FIELD(MASK(9), 6)
#define DAO_WS_HALFPERIOD(value)        BIT_FIELD((value), 6)
#define DAO_WS_HALFPERIOD_VALUE(reg) \
    FIELD_VALUE((reg), DAO_WS_HALFPERIOD_MASK, 6)
#define DAO_MUTE                        BIT(15)
/*------------------Digital Audio Input register------------------------------*/
#define DAI_WORDWIDTH_MASK              BIT_FIELD(MASK(2), 0)
#define DAI_WORDWIDTH(value)            BIT_FIELD((value), 0)
#define DAI_WORDWIDTH_VALUE(reg) \
    FIELD_VALUE((reg), DAI_WORDWIDTH_MASK, 0)
#define DAI_MONO                        BIT(2)
#define DAI_STOP                        BIT(3)
#define DAI_RESET                       BIT(4)
#define DAI_WS_SEL                      BIT(5)
#define DAI_WS_HALFPERIOD_MASK          BIT_FIELD(MASK(9), 6)
#define DAI_WS_HALFPERIOD(value)        BIT_FIELD((value), 6)
#define DAI_WS_HALFPERIOD_VALUE(reg) \
    FIELD_VALUE((reg), DAI_WS_HALFPERIOD_MASK, 6)
/*------------------Status feedback register----------------------------------*/
#define STATE_IRQ                       BIT(0)
#define STATE_DMAREQ1                   BIT(1)
#define STATE_DMAREQ2                   BIT(2)
#define STATE_RX_LEVEL_MASK             BIT_FIELD(MASK(4), 8)
#define STATE_RX_LEVEL_VALUE(reg) \
    FIELD_VALUE((reg), STATE_RX_LEVEL_MASK, 8)
#define STATE_TX_LEVEL_MASK             BIT_FIELD(MASK(4), 16)
#define STATE_TX_LEVEL_VALUE(reg) \
    FIELD_VALUE((reg), STATE_TX_LEVEL_MASK, 16)
/*------------------DMA configuration registers-------------------------------*/
#define DMA_RX_ENABLE                   BIT(0)
#define DMA_TX_ENABLE                   BIT(1)
#define DMA_RX_DEPTH_MASK               BIT_FIELD(MASK(4), 8)
#define DMA_RX_DEPTH(value)             BIT_FIELD((value), 8)
#define DMA_RX_DEPTH_VALUE(reg) \
    FIELD_VALUE((reg), DMA_RX_DEPTH_MASK, 8)
#define DMA_TX_DEPTH_MASK               BIT_FIELD(MASK(4), 16)
#define DMA_TX_DEPTH(value)             BIT_FIELD((value), 16)
#define DMA_TX_DEPTH_VALUE(reg) \
    FIELD_VALUE((reg), DMA_TX_DEPTH_MASK, 16)
/*------------------Interrupt Request Control register------------------------*/
#define IRQ_RX_ENABLE                   BIT(0)
#define IRQ_TX_ENABLE                   BIT(1)
#define IRQ_RX_DEPTH_MASK               BIT_FIELD(MASK(4), 8)
#define IRQ_RX_DEPTH(value)             BIT_FIELD((value), 8)
#define IRQ_RX_DEPTH_VALUE(reg) \
    FIELD_VALUE((reg), IRQ_RX_DEPTH_MASK, 8)
#define IRQ_TX_DEPTH_MASK               BIT_FIELD(MASK(4), 16)
#define IRQ_TX_DEPTH(value)             BIT_FIELD((value), 16)
#define IRQ_TX_DEPTH_VALUE(reg) \
    FIELD_VALUE((reg), IRQ_TX_DEPTH_MASK, 16)
/*------------------Clock rate registers--------------------------------------*/
#define RATE_Y_DIVIDER_MASK             BIT_FIELD(MASK(8), 0)
#define RATE_Y_DIVIDER(value)           BIT_FIELD((value), 0)
#define RATE_Y_DIVIDER_VALUE(reg) \
    FIELD_VALUE((reg), RATE_Y_DIVIDER_MASK, 0)
#define RATE_X_DIVIDER_MASK             BIT_FIELD(MASK(8), 8)
#define RATE_X_DIVIDER(value)           BIT_FIELD((value), 8)
#define RATE_X_DIVIDER_VALUE(reg) \
    FIELD_VALUE((reg), RATE_X_DIVIDER_MASK, 8)
/*------------------Transmit Mode control register----------------------------*/
enum
{
  TXCLKSEL_FRACTIONAL = 0,
  TXCLKSEL_RX_MCLK    = 2
};

#define TXMODE_TXCLKSEL_MASK            BIT_FIELD(MASK(2), 0)
#define TXMODE_TXCLKSEL(value)          BIT_FIELD((value), 0)
#define TXMODE_TXCLKSEL_VALUE(reg) \
    FIELD_VALUE((reg), TXMODE_TXCLKSEL_MASK, 0)
#define TXMODE_TX4PIN                   BIT(2)
#define TXMODE_TXMCENA                  BIT(3)
/*------------------Receive Mode control register-----------------------------*/
enum
{
  RXCLKSEL_FRACTIONAL = 0,
  RXCLKSEL_BASE       = 1,
  RXCLKSEL_TX_MCLK    = 2
};

#define RXMODE_RXCLKSEL_MASK            BIT_FIELD(MASK(2), 0)
#define RXMODE_RXCLKSEL(value)          BIT_FIELD((value), 0)
#define RXMODE_RXCLKSEL_VALUE(reg) \
    FIELD_VALUE((reg), RXMODE_RXCLKSEL_MASK, 0)
#define RXMODE_RX4PIN                   BIT(2)
#define RXMODE_RXMCENA                  BIT(3)
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_NXP_I2S_DEFS_H_ */
