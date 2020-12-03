/*
 * halm/platform/stm32/stm32f1xx/clocking_defs.h
 * Copyright (C) 2018 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_STM32_STM32F1XX_CLOCKING_DEFS_H_
#define HALM_PLATFORM_STM32_STM32F1XX_CLOCKING_DEFS_H_
/*----------------------------------------------------------------------------*/
#include <xcore/bits.h>
/*------------------Clock Control Register------------------------------------*/
#define CR_HSION                        BIT(0)
#define CR_HSIRDY                       BIT(1)

#define CR_HSITRIM_MASK                 BIT_FIELD(MASK(5), 3)
#define CR_HSITRIM(value)               BIT_FIELD((value), 3)
#define CR_HSITRIM_VALUE(reg)           FIELD_VALUE((reg), CR_HSITRIM_MASK, 3)

#define CR_HSICAL_MASK                  BIT_FIELD(MASK(8), 8)
#define CR_HSICAL(value)                BIT_FIELD((value), 8)
#define CR_HSICAL_VALUE(reg)            FIELD_VALUE((reg), CR_HSICAL_MASK, 8)

#define CR_HSEON                        BIT(16)
#define CR_HSERDY                       BIT(17)
#define CR_HSEBYP                       BIT(18)
#define CR_CSSON                        BIT(19)
#define CR_PLLON                        BIT(24)
#define CR_PLLRDY                       BIT(25)
#define CR_PLL2ON                       BIT(26)
#define CR_PLL2RDY                      BIT(27)
#define CR_PLL3ON                       BIT(28)
#define CR_PLL3RDY                      BIT(29)
/*------------------Clock Configuration Register------------------------------*/
enum
{
  CFGR_SW_HSI = 0x00,
  CFGR_SW_HSE = 0x01,
  CFGR_SW_PLL = 0x02
};

#define CFGR_SW_MASK                    BIT_FIELD(MASK(2), 0)
#define CFGR_SW(value)                  BIT_FIELD((value), 0)
#define CFGR_SW_VALUE(reg)              FIELD_VALUE((reg), CFGR_SW_MASK, 0)

#define CFGR_SWS_MASK                   BIT_FIELD(MASK(2), 2)
#define CFGR_SWS(value)                 BIT_FIELD((value), 2)
#define CFGR_SWS_VALUE(reg)             FIELD_VALUE((reg), CFGR_SWS_MASK, 2)

#define CFGR_HPRE_MASK                  BIT_FIELD(MASK(4), 4)
#define CFGR_HPRE(value)                BIT_FIELD((value), 4)
#define CFGR_HPRE_VALUE(reg)            FIELD_VALUE((reg), CFGR_HPRE_MASK, 4)

#define CFGR_PPRE1_MASK                 BIT_FIELD(MASK(3), 8)
#define CFGR_PPRE1(value)               BIT_FIELD((value), 8)
#define CFGR_PPRE1_VALUE(reg)           FIELD_VALUE((reg), CFGR_PPRE1_MASK, 8)

#define CFGR_PPRE2_MASK                 BIT_FIELD(MASK(3), 11)
#define CFGR_PPRE2(value)               BIT_FIELD((value), 11)
#define CFGR_PPRE2_VALUE(reg)           FIELD_VALUE((reg), CFGR_PPRE2_MASK, 11)

#define CFGR_ADCPRE_MASK                BIT_FIELD(MASK(2), 14)
#define CFGR_ADCPRE(value)              BIT_FIELD((value), 14)
#define CFGR_ADCPRE_VALUE(reg)          FIELD_VALUE((reg), CFGR_ADCPRE_MASK, 14)

#define CFGR_PLLSRC                     BIT(16)
#define CFGR_PLLXTPRE                   BIT(17)

#define CFGR_PLLMUL_MASK                BIT_FIELD(MASK(4), 18)
#define CFGR_PLLMUL(value)              BIT_FIELD((value), 18)
#define CFGR_PLLMUL_VALUE(reg)          FIELD_VALUE((reg), CFGR_PLLMUL_MASK, 18)

#define CFGR_OTGFSPRE                   BIT(22)

#define CFGR_MCO_MASK                   BIT_FIELD(MASK(4), 24)
#define CFGR_MCO(value)                 BIT_FIELD((value), 24)
#define CFGR_MCO_VALUE(reg)             FIELD_VALUE((reg), CFGR_MCO_MASK, 24)
/*------------------Clock Interrupt Register----------------------------------*/
#define CIR_LSIRDYF                     BIT(0)
#define CIR_LSERDYF                     BIT(1)
#define CIR_HSIRDYF                     BIT(2)
#define CIR_HSERDYF                     BIT(3)
#define CIR_PLLRDYF                     BIT(4)
#define CIR_PLL2RDYF                    BIT(5)
#define CIR_PLL3RDYF                    BIT(6)
#define CIR_CSSF                        BIT(7)
#define CIR_LSIRDYIE                    BIT(8)
#define CIR_LSERDYIE                    BIT(9)
#define CIR_HSIRDYIE                    BIT(10)
#define CIR_HSERDYIE                    BIT(11)
#define CIR_PLLRDYIE                    BIT(12)
#define CIR_PLL2RDYIE                   BIT(13)
#define CIR_PLL3RDYIE                   BIT(14)
#define CIR_LSIRDYC                     BIT(16)
#define CIR_LSERDYC                     BIT(17)
#define CIR_HSIRDYC                     BIT(18)
#define CIR_HSERDYC                     BIT(19)
#define CIR_PLLRDYC                     BIT(20)
#define CIR_PLL2RDYC                    BIT(21)
#define CIR_PLL3RDYC                    BIT(22)
#define CIR_CSSC                        BIT(23)
/*------------------Backup Domain Control Register----------------------------*/
#define BDCR_LSEON                      BIT(0)
#define BDCR_LSERDY                     BIT(1)
#define BDCR_LSEBYP                     BIT(2)

#define BDCR_RTCSEL_MASK                BIT_FIELD(MASK(2), 8)
#define BDCR_RTCSEL(value)              BIT_FIELD((value), 8)
#define BDCR_RTCSEL_VALUE(reg)          FIELD_VALUE((reg), BDCR_RTCSEL_MASK, 8)

#define BDCR_RTCEN                      BIT(15)
#define BDCR_BDRST                      BIT(16)
/*------------------Control/Status Register-----------------------------------*/
#define CSR_LSION                       BIT(0)
#define CSR_LSIRDY                      BIT(1)
#define CSR_RMVF                        BIT(24)
#define CSR_PINRSTF                     BIT(26)
#define CSR_PORRSTF                     BIT(27)
#define CSR_SFTRSTF                     BIT(28)
#define CSR_IWDGRSTF                    BIT(29)
#define CSR_WWDGRSTF                    BIT(30)
#define CSR_LPWRRSTF                    BIT(31)
/*------------------Clock Configuration Register 2----------------------------*/
#define CFGR2_PREDIV1_MASK              BIT_FIELD(MASK(4), 0)
#define CFGR2_PREDIV1(value)            BIT_FIELD((value), 0)
#define CFGR2_PREDIV1_VALUE(reg) \
    FIELD_VALUE((reg), CFGR2_PREDIV1_MASK, 0)

#define CFGR2_PREDIV2_MASK              BIT_FIELD(MASK(4), 4)
#define CFGR2_PREDIV2(value)            BIT_FIELD((value), 4)
#define CFGR2_PREDIV2_VALUE(reg) \
    FIELD_VALUE((reg), CFGR2_PREDIV2_MASK, 4)

#define CFGR2_PLL2MUL_MASK              BIT_FIELD(MASK(4), 8)
#define CFGR2_PLL2MUL(value)            BIT_FIELD((value), 8)
#define CFGR2_PLL2MUL_VALUE(reg) \
    FIELD_VALUE((reg), CFGR2_PLL2MUL_MASK, 8)

#define CFGR2_PLL3MUL_MASK              BIT_FIELD(MASK(4), 12)
#define CFGR2_PLL3MUL(value)            BIT_FIELD((value), 12)
#define CFGR2_PLL3MUL_VALUE(reg) \
    FIELD_VALUE((reg), CFGR2_PLL3MUL_MASK, 12)

#define CFGR2_PREDIV1SRC                BIT(16)
#define CFGR2_I2S2SRC                   BIT(17)
#define CFGR2_I2S3SRC                   BIT(18)
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_STM32_STM32F1XX_CLOCKING_DEFS_H_ */
