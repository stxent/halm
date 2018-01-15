/*
 * halm/platform/stm/spi_defs.h
 * Copyright (C) 2018 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef HALM_PLATFORM_STM_SPI_DEFS_H_
#define HALM_PLATFORM_STM_SPI_DEFS_H_
/*----------------------------------------------------------------------------*/
#include <xcore/bits.h>
/*------------------Control Register 1----------------------------------------*/
#define CR1_CPHA                        BIT(0)
#define CR1_CPOL                        BIT(1)
#define CR1_MSTR                        BIT(2)

#define CR1_BR(value)                   BIT_FIELD((value), 3)
#define CR1_BR_MASK                     BIT_FIELD(MASK(3), 3)
#define CR1_BR_VALUE(reg)               FIELD_VALUE((reg), CR1_BR_MASK, 3)

#define CR1_SPE                         BIT(6)
#define CR1_LSBFIRST                    BIT(7)
#define CR1_SSI                         BIT(8)
#define CR1_SSM                         BIT(9)
#define CR1_RXONLY                      BIT(10)
#define CR1_DFF                         BIT(11)
#define CR1_CRCNEXT                     BIT(12)
#define CR1_CRCEN                       BIT(13)
#define CR1_BIDIOE                      BIT(14)
#define CR1_BIDIMODE                    BIT(15)
/*------------------Control Register 2----------------------------------------*/
#define CR2_RXDMAEN                     BIT(0)
#define CR2_TXDMAEN                     BIT(1)
#define CR2_SSOE                        BIT(2)
#define CR2_ERRIE                       BIT(5)
#define CR2_RXNEIE                      BIT(6)
#define CR2_TXEIE                       BIT(7)
/*------------------Status Register-------------------------------------------*/
#define SR_RXNE                         BIT(0)
#define SR_TXE                          BIT(1)
#define SR_CHSIDE                       BIT(2)
#define SR_UDR                          BIT(3)
#define SR_CRCERR                       BIT(4)
#define SR_MODF                         BIT(5)
#define SR_OVR                          BIT(6)
#define SR_BSY                          BIT(7)
/*------------------I2S Configuration Register--------------------------------*/
#define I2SCFGR_CHLEN                   BIT(0)

#define I2SCFGR_DATLEN(value)           BIT_FIELD((value), 1)
#define I2SCFGR_DATLEN_MASK             BIT_FIELD(MASK(2), 1)
#define I2SCFGR_DATLEN_VALUE(reg) \
    FIELD_VALUE((reg), I2SCFGR_DATLEN_MASK, 1)

#define I2SCFGR_CKPOL                   BIT(3)

#define I2SCFGR_I2SSTD(value)           BIT_FIELD((value), 4)
#define I2SCFGR_I2SSTD_MASK             BIT_FIELD(MASK(2), 4)
#define I2SCFGR_I2SSTD_VALUE(reg) \
    FIELD_VALUE((reg), I2SCFGR_I2SSTD_MASK, 4)

#define I2SCFGR_PCMSYNC                 BIT(7)

#define I2SCFGR_I2SCFG(value)           BIT_FIELD((value), 8)
#define I2SCFGR_I2SCFG_MASK             BIT_FIELD(MASK(2), 8)
#define I2SCFGR_I2SCFG_VALUE(reg) \
    FIELD_VALUE((reg), I2SCFGR_I2SCFG_MASK, 8)

#define I2SCFGR_I2SE                    BIT(10)
#define I2SCFGR_I2SMOD                  BIT(11)
/*------------------I2S Prescaler Register------------------------------------*/
#define I2SPR_I2SDIV(value)             BIT_FIELD((value), 0)
#define I2SPR_I2SDIV_MASK               BIT_FIELD(MASK(8), 0)
#define I2SPR_I2SDIV_VALUE(reg)         FIELD_VALUE((reg), I2SPR_I2SDIV_MASK, 0)

#define I2SPR_ODD                       BIT(8)
#define I2SPR_MCKOE                     BIT(9)
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_STM_SPI_DEFS_H_ */
