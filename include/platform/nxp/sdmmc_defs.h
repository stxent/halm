/*
 * platform/nxp/sdmmc_defs.h
 * Copyright (C) 2014 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef PLATFORM_NXP_SDMMC_DEFS_H_
#define PLATFORM_NXP_SDMMC_DEFS_H_
/*----------------------------------------------------------------------------*/
#include <bits.h>
/*----------------------------------------------------------------------------*/
enum burstSize
{
  BURST_SIZE_1,
  BURST_SIZE_4,
  BURST_SIZE_8,
  BURST_SIZE_16,
  BURST_SIZE_32,
  BURST_SIZE_64,
  BURST_SIZE_128,
  BURST_SIZE_256
};

enum dmacState
{
  DMA_IDLE,
  DMA_SUSPEND,
  DMA_DESC_RD,
  DMA_DESC_CHK,
  DMA_RD_REQ_WAIT,
  DMA_WR_REQ_WAIT,
  DMA_RD,
  DMA_WR,
  DMA_DESC_CLOSE
};

enum fsmState
{
  FSM_STATE_IDLE,
  FSM_STATE_SEND_INIT_SEQUENCE,
  FSM_STATE_TX_START_BIT,
  FSM_STATE_TX_BIT,
  FSM_STATE_TX_INDEX_ARG,
  FSM_STATE_TX_CRC7,
  FSM_STATE_TX_END_BIT,
  FSM_STATE_RX_START_BIT,
  FSM_STATE_RX_IRQ_RESPONSE,
  FSM_STATE_RX_BIT,
  FSM_STATE_RX_CMD_INDEX,
  FSM_STATE_RX_DATA,
  FSM_STATE_RX_CRC7,
  FSM_STATE_RX_END_BIT,
  FSM_STATE_CMD_PATH_WAIT_NCC,
  FSM_STATE_WAIT
};
/*------------------Control register------------------------------------------*/
#define CTRL_CONTROLLER_RESET           BIT(0)
#define CTRL_FIFO_RESET                 BIT(1)
#define CTRL_DMA_RESET                  BIT(2)
#define CTRL_INT_ENABLE                 BIT(4)
#define CTRL_READ_WAIT                  BIT(6)
#define CTRL_SEND_IRQ_RESPONSE          BIT(7)
#define CTRL_ABORT_READ_DATA            BIT(8)
#define CTRL_SEND_CCSD                  BIT(9)
#define CTRL_SEND_AUTO_STOP_CCSD        BIT(10)
#define CTRL_CEATA_INT_STATUS           BIT(11)
#define CTRL_CARD_VOLTAGE_A0            BIT(16)
#define CTRL_CARD_VOLTAGE_A1            BIT(17)
#define CTRL_CARD_VOLTAGE_A2            BIT(18)
#define CTRL_USE_INTERNAL_DMAC          BIT(25)
/*------------------Power Enable register-------------------------------------*/
#define PWREN_POWER_ENABLE              BIT(0)
/*------------------Clock Divider register------------------------------------*/
#define CLKDIV_CLK_DIVIDER_MASK(channel) \
    BIT_FIELD(MASK(8), (channel) << 3)
#define CLKDIV_CLK_DIVIDER(channel, value) \
    BIT_FIELD((value), (channel) << 3)
#define CLKDIV_CLK_DIVIDER_VALUE(channel, reg) \
    FIELD_VALUE((reg), CLKDIV_CLK_DIVIDER_MASK(channel), (channel) << 3)
/*------------------Clock Source register-------------------------------------*/
#define CLKSRC_CLK_SOURCE(value)        BIT_FIELD((value), 0)
/*------------------Clock Enable register-------------------------------------*/
#define CLKENA_CCLK_ENABLE              BIT(0)
#define CLKENA_CCLK_LOW_POWER           BIT(16)
/*------------------Time-out register-----------------------------------------*/
#define TMOUT_RESPONSE_TIMEOUT_MASK     BIT_FIELD(MASK(8), 0)
#define TMOUT_RESPONSE_TIMEOUT(value)   BIT_FIELD((value), 0)
#define TMOUT_RESPONSE_TIMEOUT_VALUE(reg) \
    FIELD_VALUE((reg), TMOUT_RESPONSE_TIMEOUT_MASK, 0)
#define TMOUT_DATA_TIMEOUT_MASK         BIT_FIELD(MASK(24), 8)
#define TMOUT_DATA_TIMEOUT(value)       BIT_FIELD((value), 8)
#define TMOUT_DATA_TIMEOUT_VALUE(reg) \
    FIELD_VALUE((reg), TMOUT_DATA_TIMEOUT_MASK, 8)
/*------------------Card Type register----------------------------------------*/
#define CTYPE_CARD_WIDTH0_4BIT          BIT(0)
#define CTYPE_CARD_WIDTH0_8BIT          BIT(16)
/*------------------Interrupt Mask register-----------------------------------*/
#define INTMASK_CDET                    BIT(0)
#define INTMASK_RE                      BIT(1)
#define INTMASK_CDONE                   BIT(2)
#define INTMASK_DTO                     BIT(3)
#define INTMASK_TXDR                    BIT(4)
#define INTMASK_RXDR                    BIT(5)
#define INTMASK_RCRC                    BIT(6)
#define INTMASK_DCRC                    BIT(7)
#define INTMASK_RTO                     BIT(8)
#define INTMASK_DRTO                    BIT(9)
#define INTMASK_HTO                     BIT(10)
#define INTMASK_FRUN                    BIT(11)
#define INTMASK_HLE                     BIT(12)
#define INTMASK_SBE                     BIT(13)
#define INTMASK_ACD                     BIT(14)
#define INTMASK_EBE                     BIT(15)
#define INTMASK_SDIO_INT_MASK           BIT(16)
/*------------------Command register------------------------------------------*/
#define CMD_INDEX_MASK                  BIT_FIELD(MASK(6), 0)
#define CMD_INDEX(value)                BIT_FIELD((value), 0)
#define CMD_INDEX_VALUE(reg)            FIELD_VALUE((reg), CMD_INDEX_MASK, 0)
#define CMD_RESPONSE_EXPECT             BIT(6)
#define CMD_RESPONSE_LENGTH             BIT(7)
#define CMD_CHECK_RESPONSE_CRC          BIT(8)
#define CMD_DATA_EXPECTED               BIT(9)
#define CMD_READ_WRITE                  BIT(10)
#define CMD_TRANSFER_MODE               BIT(11)
#define CMD_SEND_AUTO_STOP              BIT(12)
#define CMD_WAIT_PRVDATA_COMPLETE       BIT(13)
#define CMD_STOP_ABORT_CMD              BIT(14)
#define CMD_SEND_INITIALIZATION         BIT(15)
#define CMD_UPDATE_CLOCK_REGISTERS      BIT(21)
#define CMD_READ_CEATA_DEVICE           BIT(22)
#define CMD_CCS_EXPECTED                BIT(23)
#define CMD_ENABLE_BOOT                 BIT(24)
#define CMD_EXPECT_BOOT_ACK             BIT(25)
#define CMD_DISABLE_BOOT                BIT(26)
#define CMD_BOOT_MODE                   BIT(27)
#define CMD_VOLT_SWITCH                 BIT(28)
#define CMD_START                       BIT(31)
/*------------------Status register-------------------------------------------*/
#define STATUS_FIFO_RX_WATERMARK        BIT(0)
#define STATUS_FIFO_TX_WATERMARK        BIT(1)
#define STATUS_FIFO_EMPTY               BIT(2)
#define STATUS_FIFO_FULL                BIT(3)
#define STATUS_CMDFSMSTATES_MASK        BIT_FIELD(MASK(4), 4)
#define STATUS_CMDFSMSTATES(reg) \
    FIELD_VALUE((reg), STATUS_CMDFSMSTATES_MASK, 4)
#define STATUS_FIFO_DATA_3              BIT(8)
#define STATUS_FIFO_DATA_BUSY           BIT(9)
#define STATUS_FIFO_DATA_STATE_MC_BUSY  BIT(10)
#define STATUS_RESPONSE_INDEX_MASK      BIT_FIELD(MASK(6), 11)
#define STATUS_RESPONSE_INDEX_VALUE(reg) \
    FIELD_VALUE((reg), STATUS_RESPONSE_INDEX_MASK, 11)
#define STATUS_FIFO_COUNT_MASK          BIT_FIELD(MASK(13), 17)
#define STATUS_FIFO_COUNT_VALUE(reg) \
    FIELD_VALUE((reg), STATUS_FIFO_COUNT_MASK, 17)
#define STATUS_FIFO_DMA_ACK             BIT(30)
#define STATUS_FIFO_DMA_REQ             BIT(31)
/*------------------FIFO Threshold Watermark register-------------------------*/
#define FIFOTH_TX_WMARK_MASK            BIT_FIELD(MASK(12), 0)
#define FIFOTH_TX_WMARK(value)          BIT_FIELD((value), 0)
#define FIFOTH_TX_WMARK_VALUE(reg) \
    FIELD_VALUE((reg), FIFOTH_TX_WMARK_MASK, 0)
#define FIFOTH_RX_WMARK_MASK            BIT_FIELD(MASK(12), 16)
#define FIFOTH_RX_WMARK(value)          BIT_FIELD((value), 16)
#define FIFOTH_RX_WMARK_VALUE(reg) \
    FIELD_VALUE((reg), FIFOTH_RX_WMARK_MASK, 16)
#define FIFOTH_DMA_MTS_MASK             BIT_FIELD(MASK(3), 28)
#define FIFOTH_DMA_MTS(value)           BIT_FIELD((value), 28)
#define FIFOTH_DMA_MTS_VALUE(reg) \
    FIELD_VALUE((reg), FIFOTH_DMA_MTS_MASK, 28)
/*------------------Bus Mode register-----------------------------------------*/
#define BMOD_SWR                        BIT(0)
#define BMOD_FB                         BIT(1)
#define BMOD_DSL_MASK                   BIT_FIELD(MASK(5), 2)
#define BMOD_DSL(value)                 BIT_FIELD((value), 2)
#define BMOD_DSL_VALUE(reg)             FIELD_VALUE((reg), BMOD_DSL_MASK, 2)
#define BMOD_DE                         BIT(7)
#define BMOD_PBL_MASK                   BIT_FIELD(MASK(3), 8)
#define BMOD_PBL(value)                 BIT_FIELD((value), 8)
#define BMOD_PBL_VALUE(reg)             FIELD_VALUE((reg), BMOD_PBL_MASK, 8)
/*------------------Internal DMAC Status register-----------------------------*/
#define IDSTS_TI                        BIT(0)
#define IDSTS_RI                        BIT(1)
#define IDSTS_FBE                       BIT(2)
#define IDSTS_DU                        BIT(4)
#define IDSTS_CES                       BIT(5)
#define IDSTS_NIS                       BIT(8)
#define IDSTS_AIS                       BIT(9)
#define IDSTS_EB_MASK                   BIT_FIELD(MASK(3), 10)
#define IDSTS_EB_VALUE(reg)             FIELD_VALUE((reg), IDSTS_EB_MASK, 10)
#define IDSTS_FSM_MASK                  BIT_FIELD(MASK(4), 13)
#define IDSTS_FSM_VALUE(reg)            FIELD_VALUE((reg), IDSTS_FSM_MASK, 13)
/*------------------Internal DMAC Interrupt Enable register-------------------*/
#define IDINTEN_TI                      BIT(0)
#define IDINTEN_RI                      BIT(1)
#define IDINTEN_FBE                     BIT(2)
#define IDINTEN_DU                      BIT(4)
#define IDINTEN_CES                     BIT(5)
#define IDINTEN_NIS                     BIT(8)
#define IDINTEN_AIS                     BIT(9)
/*----------------------------------------------------------------------------*/
#endif /* PLATFORM_NXP_SDMMC_DEFS_H_ */
