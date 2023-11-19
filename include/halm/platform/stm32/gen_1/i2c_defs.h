/*
 * halm/platform/stm32/gen_1/i2c_defs.h
 * Copyright (C) 2023 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_STM32_GEN_1_I2C_DEFS_H_
#define HALM_PLATFORM_STM32_GEN_1_I2C_DEFS_H_
/*----------------------------------------------------------------------------*/
#include <xcore/bits.h>
/*------------------Control Register 1----------------------------------------*/
#define CR1_PE                          BIT(0)
#define CR1_SMBUS                       BIT(1)
#define CR1_SMBTYPE                     BIT(3)
#define CR1_ENARP                       BIT(4)
#define CR1_ENPEC                       BIT(5)
#define CR1_ENGC                        BIT(6)
#define CR1_NOSTRETCH                   BIT(7)
#define CR1_START                       BIT(8)
#define CR1_STOP                        BIT(9)
#define CR1_ACK                         BIT(10)
#define CR1_POS                         BIT(11)
#define CR1_PEC                         BIT(12)
#define CR1_ALERT                       BIT(13)
#define CR1_SWRST                       BIT(15)
/*------------------Control Register 2----------------------------------------*/
#define CR2_FREQ_MASK                   BIT_FIELD(MASK(6), 0)
#define CR2_FREQ(value)                 BIT_FIELD((value), 0)
#define CR2_FREQ_VALUE(reg)             FIELD_VALUE((reg), CR2_FREQ_MASK, 0)

#define CR2_ITERREN                     BIT(8)
#define CR2_ITEVTEN                     BIT(9)
#define CR2_ITBUFEN                     BIT(10)
#define CR2_DMAEN                       BIT(11)
#define CR2_LAST                        BIT(12)
/*------------------Own Address Register 1------------------------------------*/
#define OAR1_ADD0                       BIT(0)

#define OAR1_ADD_MASK                   BIT_FIELD(MASK(9), 1)
#define OAR1_ADD(value)                 BIT_FIELD((value), 1)
#define OAR1_ADD_VALUE(reg)             FIELD_VALUE((reg), OAR1_ADD_MASK, 1)

/* Enable 10-bit slave address */
#define OAR1_ADDMODE                    BIT(15)
/*------------------Own Address Register 2------------------------------------*/
#define OAR2_ENDUAL                     BIT(0)

#define OAR2_ADD2_MASK                  BIT_FIELD(MASK(7), 1)
#define OAR2_ADD2(value)                BIT_FIELD((value), 1)
#define OAR2_ADD2_VALUE(reg)            FIELD_VALUE((reg), OAR2_ADD2_MASK, 1)
/*------------------Data Register---------------------------------------------*/
#define DR_READ                         BIT(0)
#define DR_WRITE                        0
/*------------------Status Register 1-----------------------------------------*/
#define SR1_SB                          BIT(0)
#define SR1_ADDR                        BIT(1)
#define SR1_BTF                         BIT(2)
#define SR1_ADD10                       BIT(3)
#define SR1_STOPF                       BIT(4)
#define SR1_RXNE                        BIT(6)
#define SR1_TXE                         BIT(7)
#define SR1_BERR                        BIT(8)
#define SR1_ARLO                        BIT(9)
#define SR1_AF                          BIT(10)
#define SR1_OVR                         BIT(11)
#define SR1_PECERR                      BIT(12)
#define SR1_TIMEOUT                     BIT(14)
#define SR1_SMBALERT                    BIT(15)
/*------------------Status Register 2-----------------------------------------*/
#define SR2_MSL                         BIT(0)
#define SR2_BUSY                        BIT(1)
#define SR2_TRA                         BIT(2)
#define SR2_GENCALL                     BIT(4)
#define SR2_SMBDEFAULT                  BIT(5)
#define SR2_SMBHOST                     BIT(6)
#define SR2_DUALF                       BIT(7)

#define SR2_PEC_MASK                    BIT_FIELD(MASK(7), 8)
#define SR2_PEC(value)                  BIT_FIELD((value), 8)
#define SR2_PEC_VALUE(reg)              FIELD_VALUE((reg), SR2_PEC_MASK, 8)
/*------------------Clock Control Register------------------------------------*/
#define CCR_CCR_MASK                    BIT_FIELD(MASK(12), 0)
#define CCR_CCR(value)                  BIT_FIELD((value), 0)
#define CCR_CCR_VALUE(reg)              FIELD_VALUE((reg), CCR_CCR_MASK, 0)
#define CCR_CCR_MAX                     MASK(12)

#define CCR_DUTY                        BIT(14)
#define CCR_FS                          BIT(15)
/*------------------TRISE register--------------------------------------------*/
#define TRISE_TRISE_MASK                BIT_FIELD(MASK(6), 0)
#define TRISE_TRISE(value)              BIT_FIELD((value), 0)
#define TRISE_TRISE_VALUE(reg)          FIELD_VALUE((reg), TRISE_TRISE_MASK, 0)
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_STM32_GEN_1_I2C_DEFS_H_ */
