/*
 * halm/platform/bouffalo/spi_defs.h
 * Copyright (C) 2026 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_BOUFFALO_SPI_DEFS_H_
#define HALM_PLATFORM_BOUFFALO_SPI_DEFS_H_
/*----------------------------------------------------------------------------*/
#include <xcore/bits.h>
/*----------------------------------------------------------------------------*/
#define SIG_COUNT                       4
#define SIG_MOSI_MISO(channel)          ((channel) * SIG_COUNT + 0)
#define SIG_MISO_MOSI(channel)          ((channel) * SIG_COUNT + 1)
#define SIG_SS(channel)                 ((channel) * SIG_COUNT + 2)
#define SIG_SCLK(channel)               ((channel) * SIG_COUNT + 3)
#define SIG_TOTAL                       (SIG_COUNT * 1)

#define SPI_FUNCTION                    4
/*------------------Configuration register------------------------------------*/
enum
{
  FSIZE_8_BIT   = 0,
  FSIZE_16_BIT  = 1,
  FSIZE_24_BIT  = 2,
  FSIZE_32_BIT  = 3
};

#define CONFIG_MEN                      BIT(0)
#define CONFIG_SEN                      BIT(1)

#define CONFIG_FSIZE(value)             BIT_FIELD((value), 2)
#define CONFIG_FSIZE_MASK               BIT_FIELD(MASK(2), 2)
#define CONFIG_FSIZE_VALUE(reg)         FIELD_VALUE((reg), CONFIG_FSIZE_MASK, 2)

#define CONFIG_SCLKPOL                  BIT(4)
#define CONFIG_SCLKPH                   BIT(5)
#define CONFIG_BITINV                   BIT(6)
#define CONFIG_BYTEINV                  BIT(7)
#define CONFIG_IGNREN                   BIT(8)
#define CONFIG_MCEN                     BIT(9)
#define CONFIG_DEGEN                    BIT(11)

#define CONFIG_DEGCNT(value)            BIT_FIELD((value), 12)
#define CONFIG_DEGCNT_MASK              BIT_FIELD(MASK(4), 12)
#define CONFIG_DEGCNT_VALUE(reg) \
    FIELD_VALUE((reg), CONFIG_DEGCNT_MASK, 12)
/*------------------Interrupt Status register---------------------------------*/
#define INT_STS_ENDINT                  BIT(0) /* Transfer end interrupt */
#define INT_STS_TXFINT                  BIT(1) /* TX FIFO ready */
#define INT_STS_RXFINT                  BIT(2) /* RX FIFO ready */
#define INT_STS_STOINT                  BIT(3) /* Slave mode transfer timeout */
#define INT_STS_TXUINT                  BIT(4) /* Slave mode TX underrun */
#define INT_STS_FERINT                  BIT(5) /* TX/RX FIFO error */
#define INT_STS_ENDMASK                 BIT(8)
#define INT_STS_TXFMASK                 BIT(9)
#define INT_STS_RXFMASK                 BIT(10)
#define INT_STS_STOMASK                 BIT(11)
#define INT_STS_TXUMASK                 BIT(12)
#define INT_STS_FERMASK                 BIT(13)
#define INT_STS_ENDCLR                  BIT(16)
#define INT_STS_STOCLR                  BIT(19)
#define INT_STS_TXUCLR                  BIT(20)
#define INT_STS_ENDEN                   BIT(24)
#define INT_STS_TXFEN                   BIT(25)
#define INT_STS_RXFEN                   BIT(26)
#define INT_STS_STOEN                   BIT(27)
#define INT_STS_TXUEN                   BIT(28)
#define INT_STS_FEREN                   BIT(29)

#define INT_STS_CLR_ALL \
    (INT_STS_ENDCLR | INT_STS_STOCLR | INT_STS_TXUCLR)
#define INT_STS_MASK_ALL                BIT_FIELD(MASK(6), 8)
/*------------------Bus Busy register-----------------------------------------*/
#define BUS_BUSY_BUSBUSY                BIT(0)
/*------------------Length control register-----------------------------------*/
#define PRD0_PRDS(value)                BIT_FIELD((value), 0)
#define PRD0_PRDS_MASK                  BIT_FIELD(MASK(8), 0)
#define PRD0_PRDS_VALUE(reg)            FIELD_VALUE((reg), PRD0_PRDS_MASK, 0)

#define PRD0_PRDP(value)                BIT_FIELD((value), 8)
#define PRD0_PRDP_MASK                  BIT_FIELD(MASK(8), 8)
#define PRD0_PRDP_VALUE(reg)            FIELD_VALUE((reg), PRD0_PRDP_MASK, 8)

#define PRD0_PRDPH0(value)              BIT_FIELD((value), 16)
#define PRD0_PRDPH0_MASK                BIT_FIELD(MASK(8), 16)
#define PRD0_PRDPH0_VALUE(reg)          FIELD_VALUE((reg), PRD0_PRDPH0_MASK, 16)

#define PRD0_PRDPH1(value)              BIT_FIELD((value), 24)
#define PRD0_PRDPH1_MASK                BIT_FIELD(MASK(8), 24)
#define PRD0_PRDPH1_VALUE(reg)          FIELD_VALUE((reg), PRD0_PRDPH1_MASK, 24)

#define PRD0_PRD_MAX                    255
/*------------------Length of interval register-------------------------------*/
#define PRD1_PRDI(value)                BIT_FIELD((value), 0)
#define PRD1_PRDI_MASK                  BIT_FIELD(MASK(8), 0)
#define PRD1_PRDI_VALUE(reg)            FIELD_VALUE((reg), PRD1_PRDI_MASK, 0)

#define PRD1_PRD_MAX                    255
/*------------------Ignore function register----------------------------------*/
#define RXD_IGNR_RXDIGP(value)          BIT_FIELD((value), 0)
#define RXD_IGNR_RXDIGP_MASK            BIT_FIELD(MASK(5), 0)
#define RXD_IGNR_RXDIGP_VALUE(reg) \
    FIELD_VALUE((reg), RXD_IGNR_RXDIGP_MASK, 0)

#define RXD_IGNR_RXDIGS(value)          BIT_FIELD((value), 16)
#define RXD_IGNR_RXDIGS_MASK            BIT_FIELD(MASK(5), 16)
#define RXD_IGNR_RXDIGS_VALUE(reg) \
    FIELD_VALUE((reg), RXD_IGNR_RXDIGS_MASK, 16)
/*------------------Time-out value register-----------------------------------*/
#define STO_VALUE_STOV(value)           BIT_FIELD((value), 0)
#define STO_VALUE_STOV_MASK             BIT_FIELD(MASK(12), 0)
#define STO_VALUE_STOV_VALUE(reg) \
    FIELD_VALUE((reg), STO_VALUE_STOV_MASK, 0)
/*------------------FIFO Configuration 0 register-----------------------------*/
#define FIFO_CONFIG0_DMATEN             BIT(0) /* Enable DMA TX requests */
#define FIFO_CONFIG0_DMAREN             BIT(1) /* Enable DMA RX requests */
#define FIFO_CONFIG0_TFC                BIT(2) /* Clear signal of TX FIFO */
#define FIFO_CONFIG0_RFC                BIT(3) /* Clear signal of RX FIFO */
#define FIFO_CONFIG0_TFOF               BIT(4) /* Overflow flag of TX FIFO */
#define FIFO_CONFIG0_TFUF               BIT(5) /* Underflow flag of TX FIFO */
#define FIFO_CONFIG0_RFOF               BIT(6) /* Overflow flag of RX FIFO */
#define FIFO_CONFIG0_RFUF               BIT(7) /* Underflow flag of RX FIFO */
/*------------------FIFO Configuration 1 register-----------------------------*/
#define FIFO_CONFIG1_TFCNT(value)       BIT_FIELD((value), 0)
#define FIFO_CONFIG1_TFCNT_MASK         BIT_FIELD(MASK(3), 0)
#define FIFO_CONFIG1_TFCNT_VALUE(reg) \
    FIELD_VALUE((reg), FIFO_CONFIG1_TFCNT_MASK, 0)

#define FIFO_CONFIG1_RFCNT(value)       BIT_FIELD((value), 8)
#define FIFO_CONFIG1_RFCNT_MASK         BIT_FIELD(MASK(3), 8)
#define FIFO_CONFIG1_RFCNT_VALUE(reg) \
    FIELD_VALUE((reg), FIFO_CONFIG1_RFCNT_MASK, 8)

#define FIFO_CONFIG1_TFTH(value)        BIT_FIELD((value), 16)
#define FIFO_CONFIG1_TFTH_MASK          BIT_FIELD(MASK(2), 16)
#define FIFO_CONFIG1_TFTH_VALUE(reg) \
    FIELD_VALUE((reg), FIFO_CONFIG1_TFTH_MASK, 16)

#define FIFO_CONFIG1_RFTH(value)        BIT_FIELD((value), 24)
#define FIFO_CONFIG1_RFTH_MASK          BIT_FIELD(MASK(2), 24)
#define FIFO_CONFIG1_RFTH_VALUE(reg) \
    FIELD_VALUE((reg), FIFO_CONFIG1_RFTH_MASK, 24)

#define FIFO_CONFIG1_FTH_MAX            3
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_BOUFFALO_SPI_DEFS_H_ */
