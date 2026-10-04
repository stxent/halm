/*
 * halm/platform/bouffalo/i2c_defs.h
 * Copyright (C) 2026 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_BOUFFALO_I2C_DEFS_H_
#define HALM_PLATFORM_BOUFFALO_I2C_DEFS_H_
/*----------------------------------------------------------------------------*/
#include <xcore/bits.h>
/*----------------------------------------------------------------------------*/
/* I2C pin function */
#define PIN_I2C_FUNCTION                6
/*------------------Configuration register------------------------------------*/
/*
 * Enable signal of I2C Master function. Asserting this bit will trigger
 * the transaction, and should be de-asserted after finish.
 */
#define CONFIG_MEN                      BIT(0)
/* Transfer direction of the packet. 1'b0: Write; 1'b1: Read */
#define CONFIG_PKTDIR                   BIT(1)
/* Enable signal of I2C input de-glitch function (for all input pins) */
#define CONFIG_DEGEN                    BIT(2)
/*
 * Enable signal of I2C SCL synchronization, should be enabled to support
 * Multi-Master and Clock-Stretching (Normally should not be turned-off).
 */
#define CONFIG_SCLSEN                   BIT(3)
/* Enable signal of I2C sub-address field */
#define CONFIG_SAEN                     BIT(4)
/* Sub-address field byte count */
#define CONFIG_SABC_MASK                BIT_FIELD(MASK(2), 5)
#define CONFIG_SABC(value)              BIT_FIELD(value, 5)
#define CONFIG_SABC_VALUE(reg)          FIELD_VALUE(reg, CONFIG_SABC_MASK, 5)

/* Slave address for I2C transaction (target address) */
#define CONFIG_SLVADDR_MASK             BIT_FIELD(MASK(7), 8)
#define CONFIG_SLVADDR(value)           BIT_FIELD(value, 8)
#define CONFIG_SLVADDR_VALUE(reg)       FIELD_VALUE(reg, CONFIG_SLVADDR_MASK, 8)

/* Packet length (unit: byte) */
#define CONFIG_PKTLEN_MASK              BIT_FIELD(MASK(8), 16)
#define CONFIG_PKTLEN(value)            BIT_FIELD(value, 16)
#define CONFIG_PKTLEN_VALUE(reg)        FIELD_VALUE(reg, CONFIG_PKTLEN_MASK, 16)

/* De-glitch function cycle count */
#define CONFIG_DEGCNT_MASK              BIT_FIELD(MASK(4), 28)
#define CONFIG_DEGCNT(value)            BIT_FIELD(value, 28)
#define CONFIG_DEGCNT_VALUE(reg)        FIELD_VALUE(reg, CONFIG_DEGCNT_MASK, 28)
/*------------------Interrupt Status register---------------------------------*/
/* I2C transfer end interrupt */
#define INT_STS_ENDINT                  BIT(0)
/* I2C TX FIFO ready (tx_fifo_cnt > tx_fifo_th) interrupt, auto-cleared */
#define INT_STS_TXFINT                  BIT(1)
/* I2C RX FIFO ready (rx_fifo_cnt > rx_fifo_th) interrupt, auto-cleared */
#define INT_STS_RXFINT                  BIT(2)
/* I2C NACK-received interrupt */
#define INT_STS_NAKINT                  BIT(3)
/* I2C arbitration lost interrupt */
#define INT_STS_ARBINT                  BIT(4)
/* I2C TX/RX FIFO error interrupt, set on FIFO overflow/underflow errors */
#define INT_STS_FERINT                  BIT(5)
/* Interrupt mask of i2c_end_int */
#define INT_STS_ENDMASK                 BIT(8)
/* Interrupt mask of i2c_txf_int */
#define INT_STS_TXFMASK                 BIT(9)
/* Interrupt mask of i2c_rxf_int */
#define INT_STS_RXFMASK                 BIT(10)
/* Interrupt mask of i2c_nak_int */
#define INT_STS_NAKMASK                 BIT(11)
/* Interrupt mask of i2c_arb_int */
#define INT_STS_ARBMASK                 BIT(12)
/* Interrupt mask of i2c_fer_int */
#define INT_STS_FERMASK                 BIT(13)
/* Interrupt clear of i2c_end_int */
#define INT_STS_ENDCLR                  BIT(16)
/* Interrupt clear of i2c_nak_int */
#define INT_STS_NAKCLR                  BIT(19)
/* Interrupt clear of i2c_arb_int */
#define INT_STS_ARBCLR                  BIT(20)
/* Interrupt enable of i2c_end_int */
#define INT_STS_ENDEN                   BIT(24)
/* Interrupt enable of i2c_txf_int */
#define INT_STS_TXFEN                   BIT(25)
/* Interrupt enable of i2c_rxf_int */
#define INT_STS_RXFEN                   BIT(26)
/* Interrupt enable of i2c_nak_int */
#define INT_STS_NAKEN                   BIT(27)
/* Interrupt enable of i2c_arb_int */
#define INT_STS_ARBEN                   BIT(28)
/* Interrupt enable of i2c_fer_int */
#define INT_STS_FEREN                   BIT(29)
/*------------------Sub-Address configuration register------------------------*/
/* I2C sub-address field - byte[0] (sub-address starts from this byte) */
#define SUB_ADDR_SUBAB0_MASK            BIT_FIELD(MASK(8), 0)
#define SUB_ADDR_SUBAB0(value)          BIT_FIELD(value, 0)
#define SUB_ADDR_SUBAB0_VALUE(reg) \
    FIELD_VALUE(reg, SUB_ADDR_SUBAB0_MASK, 0)

/* I2C sub-address field - byte[1] */
#define SUB_ADDR_SUBAB1_MASK            BIT_FIELD(MASK(8), 8)
#define SUB_ADDR_SUBAB1(value)          BIT_FIELD(value, 8)
#define SUB_ADDR_SUBAB1_VALUE(reg) \
    FIELD_VALUE(reg, SUB_ADDR_SUBAB1_MASK, 8)

/* I2C sub-address field - byte[2] */
#define SUB_ADDR_SUBAB2_MASK            BIT_FIELD(MASK(8), 16)
#define SUB_ADDR_SUBAB2(value)          BIT_FIELD(value, 16)
#define SUB_ADDR_SUBAB2_VALUE(reg) \
    FIELD_VALUE(reg, SUB_ADDR_SUBAB2_MASK, 16)

/* I2C sub-address field - byte[3] */
#define SUB_ADDR_SUBAB3_MASK            BIT_FIELD(MASK(8), 24)
#define SUB_ADDR_SUBAB3(value)          BIT_FIELD(value, 24)
#define SUB_ADDR_SUBAB3_VALUE(reg) \
    FIELD_VALUE(reg, SUB_ADDR_SUBAB3_MASK, 24)
/*------------------Bus Busy Control register---------------------------------*/
/* Indicator of I2C bus busy */
#define BUS_BUSY_BUSY                   BIT(0)
/* Clear signal of bus_busy status, in case I2C bus */
#define BUS_BUSY_BUSYCLR                BIT(1)
/*------------------Length of start phase-------------------------------------*/
/* Length of START condition phase 0 */
#define PRD_START_PRDSPH0_MASK          BIT_FIELD(MASK(8), 0)
#define PRD_START_PRDSPH0(value)        BIT_FIELD(value, 0)
#define PRD_START_PRDSPH0_VALUE(reg) \
    FIELD_VALUE(reg, PRD_START_PRDSPH0_MASK, 0)

/* Length of START condition phase 1 */
#define PRD_START_PRDSPH1_MASK          BIT_FIELD(MASK(8), 8)
#define PRD_START_PRDSPH1(value)        BIT_FIELD(value, 8)
#define PRD_START_PRDSPH1_VALUE(reg) \
    FIELD_VALUE(reg, PRD_START_PRDSPH1_MASK, 8)

/* Length of START condition phase 2 */
#define PRD_START_PRDSPH2_MASK          BIT_FIELD(MASK(8), 16)
#define PRD_START_PRDSPH2(value)        BIT_FIELD(value, 16)
#define PRD_START_PRDSPH2_VALUE(reg) \
    FIELD_VALUE(reg, PRD_START_PRDSPH2_MASK, 16)

/* Length of START condition phase 3 */
#define PRD_START_PRDSPH3_MASK          BIT_FIELD(MASK(8), 24)
#define PRD_START_PRDSPH3(value)        BIT_FIELD(value, 24)
#define PRD_START_PRDSPH3_VALUE(reg) \
    FIELD_VALUE(reg, PRD_START_PRDSPH3_MASK, 24)
/*------------------Length of stop phase--------------------------------------*/
/* Length of STOP condition phase 0 */
#define PRD_STOP_PRDPPH0_MASK           BIT_FIELD(MASK(8), 0)
#define PRD_STOP_PRDPPH0(value)         BIT_FIELD(value, 0)
#define PRD_STOP_PRDPPH0_VALUE(reg) \
    FIELD_VALUE(reg, PRD_STOP_PRDPPH0_MASK, 0)

/* Length of STOP condition phase 1 */
#define PRD_STOP_PRDPPH1_MASK           BIT_FIELD(MASK(8), 8)
#define PRD_STOP_PRDPPH1(value)         BIT_FIELD(value, 8)
#define PRD_STOP_PRDPPH1_VALUE(reg) \
    FIELD_VALUE(reg, PRD_STOP_PRDPPH1_MASK, 8)

/* Length of STOP condition phase 2 */
#define PRD_STOP_PRDPPH2_MASK           BIT_FIELD(MASK(8), 16)
#define PRD_STOP_PRDPPH2(value)         BIT_FIELD(value, 16)
#define PRD_STOP_PRDPPH2_VALUE(reg) \
    FIELD_VALUE(reg, PRD_STOP_PRDPPH2_MASK, 16)

/* Length of STOP condition phase 3 */
#define PRD_STOP_PRDPPH3_MASK           BIT_FIELD(MASK(8), 24)
#define PRD_STOP_PRDPPH3(value)         BIT_FIELD(value, 24)
#define PRD_STOP_PRDPPH3_VALUE(reg) \
    FIELD_VALUE(reg, PRD_STOP_PRDPPH3_MASK, 24)
/*------------------Length of data phase--------------------------------------*/
/* Length of DATA phase 0 */
#define PRD_DATA_PRDDPH0_MASK           BIT_FIELD(MASK(8), 0)
#define PRD_DATA_PRDDPH0(value)         BIT_FIELD(value, 0)
#define PRD_DATA_PRDDPH0_VALUE(reg) \
    FIELD_VALUE(reg, PRD_DATA_PRDDPH0_MASK, 0)

/*
 * Length of DATA phase 1. Note: This value should not be set to 8'd0,
 * adjust source clock rate instead if higher I2C clock rate is required.
 */
#define PRD_DATA_PRDDPH1_MASK           BIT_FIELD(MASK(8), 8)
#define PRD_DATA_PRDDPH1(value)         BIT_FIELD(value, 8)
#define PRD_DATA_PRDDPH1_VALUE(reg) \
    FIELD_VALUE(reg, PRD_DATA_PRDDPH1_MASK, 8)

/* Length of DATA phase 2 */
#define PRD_DATA_PRDDPH2_MASK           BIT_FIELD(MASK(8), 16)
#define PRD_DATA_PRDDPH2(value)         BIT_FIELD(value, 16)
#define PRD_DATA_PRDDPH2_VALUE(reg) \
    FIELD_VALUE(reg, PRD_DATA_PRDDPH2_MASK, 16)

/* Length of DATA phase 3 */
#define PRD_DATA_PRDDPH3_MASK           BIT_FIELD(MASK(8), 24)
#define PRD_DATA_PRDDPH3(value)         BIT_FIELD(value, 24)
#define PRD_DATA_PRDDPH3_VALUE(reg) \
    FIELD_VALUE(reg, PRD_DATA_PRDDPH3_MASK, 24)
/*------------------FIFO Configuration register 0-----------------------------*/
/* Enable signal of dma_tx_req/ack interface */
#define FIFO_CONFIG_0_DTEN              BIT(0)
/* Enable signal of dma_rx_req/ack interface */
#define FIFO_CONFIG_0_DREN              BIT(1)
/* Clear signal of TX FIFO */
#define FIFO_CONFIG_0_TFICLR            BIT(2)
/* Clear signal of RX FIFO */
#define FIFO_CONFIG_0_RFICLR            BIT(3)
/* Overflow flag of TX FIFO, can be cleared by tx_fifo_clr */
#define FIFO_CONFIG_0_TFIO              BIT(4)
/* Underflow flag of TX FIFO, can be cleared by tx_fifo_clr */
#define FIFO_CONFIG_0_TFIU              BIT(5)
/* Overflow flag of RX FIFO, can be cleared by rx_fifo_clr */
#define FIFO_CONFIG_0_RFIO              BIT(6)
/* Underflow flag of RX FIFO, can be cleared by rx_fifo_clr */
#define FIFO_CONFIG_0_RFIU              BIT(7)
/*------------------FIFO Configuration register 1-----------------------------*/
/* TX FIFO available count */
#define FIFO_CONFIG_1_TFICNT_MASK       BIT_FIELD(MASK(2), 0)
#define FIFO_CONFIG_1_TFICNT(value)     BIT_FIELD(value, 0)
#define FIFO_CONFIG_1_TFICNT_VALUE(reg) \
    FIELD_VALUE(reg, FIFO_CONFIG_1_TFICNT_MASK, 0)

/* RX FIFO available count */
#define FIFO_CONFIG_1_RFICNT_MASK       BIT_FIELD(MASK(2), 8)
#define FIFO_CONFIG_1_RFICNT(value)     BIT_FIELD(value, 8)
#define FIFO_CONFIG_1_RFICNT_VALUE(reg) \
    FIELD_VALUE(reg, FIFO_CONFIG_1_RFICNT_MASK, 8)

/* TX FIFO threshold */
#define FIFO_CONFIG_1_TFITH             BIT(16)
/* RX FIFO threshold */
#define FIFO_CONFIG_1_RFITH             BIT(24)
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_BOUFFALO_I2C_DEFS_H_ */
