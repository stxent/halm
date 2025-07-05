/*
 * halm/platform/stm32/gen_2/i2c_defs.h
 * Copyright (C) 2023 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_STM32_GEN_2_I2C_DEFS_H_
#define HALM_PLATFORM_STM32_GEN_2_I2C_DEFS_H_
/*----------------------------------------------------------------------------*/
#include <xcore/bits.h>
/*------------------Control Register 1----------------------------------------*/
#define CR1_PE                          BIT(0)
#define CR1_TXIE                        BIT(1)
#define CR1_RXIE                        BIT(2)
#define CR1_ADDRIE                      BIT(3)
#define CR1_NACKIE                      BIT(4)
#define CR1_STOPIE                      BIT(5)
#define CR1_TCIE                        BIT(6)
#define CR1_ERRIE                       BIT(7)

#define CR1_DNF_MASK                    BIT_FIELD(MASK(4), 8)
#define CR1_DNF(value)                  BIT_FIELD((value), 8)
#define CR1_DNF_VALUE(reg)              FIELD_VALUE((reg), CR1_DNF_MASK, 8)

#define CR1_ANFOFF                      BIT(12)
#define CR1_TXDMAEN                     BIT(14)
#define CR1_RXDMAEN                     BIT(15)
#define CR1_SBC                         BIT(16)
#define CR1_NOSTRETCH                   BIT(17)
#define CR1_GCEN                        BIT(19)
#define CR1_SMBHEN                      BIT(20)
#define CR1_SMBDEN                      BIT(21)
#define CR1_ALERTEN                     BIT(22)
#define CR1_PECEN                       BIT(23)
/*------------------Control Register 2----------------------------------------*/
#define CR2_SADD10_MASK                 BIT_FIELD(MASK(10), 0)
#define CR2_SADD10(value)               BIT_FIELD((value), 0)
#define CR2_SADD10_VALUE(reg)           FIELD_VALUE((reg), CR2_SADD10_MASK, 0)

#define CR2_SADD7_MASK                  BIT_FIELD(MASK(7), 1)
#define CR2_SADD7(value)                BIT_FIELD((value), 1)
#define CR2_SADD7_VALUE(reg)            FIELD_VALUE((reg), CR2_SADD7_MASK, 1)

#define CR2_RD_WRN                      BIT(10)
#define CR2_ADD10                       BIT(11)
#define CR2_HEAD10R                     BIT(12)
#define CR2_START                       BIT(13)
#define CR2_STOP                        BIT(14)
#define CR2_NACK                        BIT(15)

#define CR2_NBYTES_MASK                 BIT_FIELD(MASK(8), 16)
#define CR2_NBYTES(value)               BIT_FIELD((value), 16)
#define CR2_NBYTES_VALUE(reg)           FIELD_VALUE((reg), CR2_NBYTES_MASK, 16)
#define CR2_NBYTES_MAX                  255

#define CR2_RELOAD                      BIT(24)
#define CR2_AUTOEND                     BIT(25)
#define CR2_PECBYTE                     BIT(26)
/*------------------Own Address Register 1------------------------------------*/
#define OAR1_OA1_ADD0                   BIT(0)

#define OAR1_OA1_MASK                   BIT_FIELD(MASK(9), 1)
#define OAR1_OA1(value)                 BIT_FIELD((value), 1)
#define OAR1_OA1_VALUE(reg)             FIELD_VALUE((reg), OAR1_OA1_MASK, 1)

/* Enable 10-bit slave address */
#define OAR1_OA1MODE                    BIT(10)

#define OAR1_OA1EN                      BIT(15)
/*------------------Own Address Register 2------------------------------------*/
#define OAR2_OA2_MASK                   BIT_FIELD(MASK(7), 1)
#define OAR2_OA2(value)                 BIT_FIELD((value), 1)
#define OAR2_OA2_VALUE(reg)             FIELD_VALUE((reg), OAR2_OA2_MASK, 1)

#define OAR2_OA2MSK_MASK                BIT_FIELD(MASK(3), 8)
#define OAR2_OA2MSK(value)              BIT_FIELD((value), 8)
#define OAR2_OA2MSK_VALUE(reg)          FIELD_VALUE((reg), OAR2_OA2MSK_MASK, 8)

#define OAR2_OA2EN                      BIT(15)
/*------------------Timing Register-------------------------------------------*/
#define TIMINGR_SCLL_MASK               BIT_FIELD(MASK(8), 0)
#define TIMINGR_SCLL(value)             BIT_FIELD((value), 0)
#define TIMINGR_SCLL_VALUE(reg)         FIELD_VALUE((reg), TIMINGR_SCLL_MASK, 0)
#define TIMINGR_SCLL_MAX                (MASK(8) + 1)

#define TIMINGR_SCLH_MASK               BIT_FIELD(MASK(8), 8)
#define TIMINGR_SCLH(value)             BIT_FIELD((value), 8)
#define TIMINGR_SCLH_VALUE(reg)         FIELD_VALUE((reg), TIMINGR_SCLH_MASK, 8)
#define TIMINGR_SCLH_MAX                (MASK(8) + 1)

#define TIMINGR_SDADEL_MASK             BIT_FIELD(MASK(4), 16)
#define TIMINGR_SDADEL(value)           BIT_FIELD((value), 16)
#define TIMINGR_SDADEL_VALUE(reg) \
    FIELD_VALUE((reg), TIMINGR_SDADEL_MASK, 16)
#define TIMINGR_SDADEL_MAX              MASK(4)

#define TIMINGR_SCLDEL_MASK             BIT_FIELD(MASK(4), 20)
#define TIMINGR_SCLDEL(value)           BIT_FIELD((value), 20)
#define TIMINGR_SCLDEL_VALUE(reg) \
    FIELD_VALUE((reg), TIMINGR_SCLDEL_MASK, 20)
#define TIMINGR_SCLDEL_MAX              (MASK(4) + 1)

#define TIMINGR_PRESC_MASK              BIT_FIELD(MASK(4), 28)
#define TIMINGR_PRESC(value)            BIT_FIELD((value), 28)
#define TIMINGR_PRESC_VALUE(reg) \
    FIELD_VALUE((reg), TIMINGR_PRESC_MASK, 28)
#define TIMINGR_PRESC_MAX               (MASK(4) + 1)
/*------------------Timeout Register------------------------------------------*/
#define TIMEOUTR_TIMEOUTA_MASK          BIT_FIELD(MASK(12), 0)
#define TIMEOUTR_TIMEOUTA(value)        BIT_FIELD((value), 0)
#define TIMEOUTR_TIMEOUTA_VALUE(reg) \
    FIELD_VALUE((reg), TIMEOUTR_TIMEOUTA_MASK, 0)

#define TIMEOUTR_TIDLE                  BIT(12)
#define TIMEOUTR_TIMOUTEN               BIT(15)

#define TIMEOUTR_TIMEOUTB_MASK          BIT_FIELD(MASK(12), 16)
#define TIMEOUTR_TIMEOUTB(value)        BIT_FIELD((value), 16)
#define TIMEOUTR_TIMEOUTB_VALUE(reg) \
    FIELD_VALUE((reg), TIMEOUTR_TIMEOUTB_MASK, 16)

#define TIMEOUTR_TEXTEN                 BIT(31)
/*------------------Interrupt and Status Register-----------------------------*/
#define ISR_TXE                         BIT(0)
#define ISR_TXIS                        BIT(1)
#define ISR_RXNE                        BIT(2)
#define ISR_ADDR                        BIT(3)
#define ISR_NACKF                       BIT(4)
#define ISR_STOPF                       BIT(5)
#define ISR_TC                          BIT(6)
#define ISR_TCR                         BIT(7)
#define ISR_BERR                        BIT(8)
#define ISR_ARLO                        BIT(9)
#define ISR_OVR                         BIT(10)
#define ISR_PECERR                      BIT(11)
#define ISR_TIMEOUT                     BIT(12)
#define ISR_ALERT                       BIT(13)
#define ISR_BUSY                        BIT(15)
#define ISR_DIR                         BIT(16)

#define ISR_ADDCODE_MASK                BIT_FIELD(MASK(7), 17)
#define ISR_ADDCODE(value)              BIT_FIELD((value), 17)
#define ISR_ADDCODE_VALUE(reg)          FIELD_VALUE((reg), ISR_ADDCODE_MASK, 17)
/*------------------Interrupt Clear Register----------------------------------*/
#define ICR_ADDRCF                      BIT(3)
#define ICR_NACKCF                      BIT(4)
#define ICR_STOPCF                      BIT(5)
#define ICR_BERRCF                      BIT(8)
#define ICR_ARLOCF                      BIT(9)
#define ICR_OVRCF                       BIT(10)
#define ICR_PECCF                       BIT(11)
#define ICR_TIMOUTCF                    BIT(12)
#define ICR_ALERTCF                     BIT(13)
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_STM32_GEN_2_I2C_DEFS_H_ */
