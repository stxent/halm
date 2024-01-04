/*
 * halm/platform/stm32/stm32f4xx/clocking_defs.h
 * Copyright (C) 2024 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_STM32_STM32F4XX_CLOCKING_DEFS_H_
#define HALM_PLATFORM_STM32_STM32F4XX_CLOCKING_DEFS_H_
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
#define CR_PLLI2SON                     BIT(26)
#define CR_PLLI2S2RDY                   BIT(27)
/*------------------PLL Configuration Register--------------------------------*/
#define PLLCFGR_PLLM_MASK               BIT_FIELD(MASK(6), 0)
#define PLLCFGR_PLLM(value)             BIT_FIELD((value), 0)
#define PLLCFGR_PLLM_VALUE(reg)         FIELD_VALUE((reg), PLLCFGR_PLLM_MASK, 0)
#define PLLCFGR_PLLM_MAX                63
#define PLLCFGR_PLLM_MIN                2

#define PLLCFGR_PLLN_MASK               BIT_FIELD(MASK(9), 6)
#define PLLCFGR_PLLN(value)             BIT_FIELD((value), 6)
#define PLLCFGR_PLLN_VALUE(reg)         FIELD_VALUE((reg), PLLCFGR_PLLN_MASK, 6)
#define PLLCFGR_PLLN_MAX                432
#define PLLCFGR_PLLN_MIN                64

#define PLLCFGR_PLLP_MASK               BIT_FIELD(MASK(2), 16)
#define PLLCFGR_PLLP(value)             BIT_FIELD((value), 16)
#define PLLCFGR_PLLP_VALUE(reg) \
    FIELD_VALUE((reg), PLLCFGR_PLLP_MASK, 16)

/* Select HSE as a clock source for PLL and PLLI2S */
#define PLLCFGR_PLLSRC                  BIT(22)

#define PLLCFGR_PLLQ_MASK               BIT_FIELD(MASK(4), 24)
#define PLLCFGR_PLLQ(value)             BIT_FIELD((value), 24)
#define PLLCFGR_PLLQ_VALUE(reg) \
    FIELD_VALUE((reg), PLLCFGR_PLLQ_MASK, 24)
#define PLLCFGR_PLLQ_MAX                15
#define PLLCFGR_PLLQ_MIN                2
/*------------------Clock Configuration Register------------------------------*/
enum
{
  CFGR_SW_HSI = 0x00,
  CFGR_SW_HSE = 0x01,
  CFGR_SW_PLL = 0x02
};

enum
{
  CFGR_MCO1_HSI,
  CFGR_MCO1_LSE,
  CFGR_MCO1_HSE,
  CFGR_MCO1_PLL
};

enum
{
  CFGR_MCO2_SYSCLK,
  CFGR_MCO2_PLLI2S,
  CFGR_MCO2_HSE,
  CFGR_MCO2_PLL
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

#define CFGR_PPRE1_MASK                 BIT_FIELD(MASK(3), 10)
#define CFGR_PPRE1(value)               BIT_FIELD((value), 10)
#define CFGR_PPRE1_VALUE(reg)           FIELD_VALUE((reg), CFGR_PPRE1_MASK, 10)

#define CFGR_PPRE2_MASK                 BIT_FIELD(MASK(3), 13)
#define CFGR_PPRE2(value)               BIT_FIELD((value), 13)
#define CFGR_PPRE2_VALUE(reg)           FIELD_VALUE((reg), CFGR_PPRE2_MASK, 13)

#define CFGR_RTCPRE_MASK                BIT_FIELD(MASK(5), 16)
#define CFGR_RTCPRE(value)              BIT_FIELD((value), 16)
#define CFGR_RTCPRE_VALUE(reg)          FIELD_VALUE((reg), CFGR_RTCPRE_MASK, 16)

#define CFGR_MCO1_MASK                  BIT_FIELD(MASK(2), 21)
#define CFGR_MCO1(value)                BIT_FIELD((value), 21)
#define CFGR_MCO1_VALUE(reg)            FIELD_VALUE((reg), CFGR_MCO1_MASK, 21)

/* Select external I2S clock as I2S clock source */
#define CFGR_I2SSRC                     BIT(23)

#define CFGR_MCO1PRE_MASK               BIT_FIELD(MASK(3), 24)
#define CFGR_MCO1PRE(value)             BIT_FIELD((value), 24)
#define CFGR_MCO1PRE_VALUE(reg) \
    FIELD_VALUE((reg), CFGR_MCO1PRE_MASK, 24)

#define CFGR_MCO2PRE_MASK               BIT_FIELD(MASK(3), 27)
#define CFGR_MCO2PRE(value)             BIT_FIELD((value), 27)
#define CFGR_MCO2PRE_VALUE(reg) \
    FIELD_VALUE((reg), CFGR_MCO2PRE_MASK, 27)

#define CFGR_MCO2_MASK                  BIT_FIELD(MASK(2), 30)
#define CFGR_MCO2(value)                BIT_FIELD((value), 30)
#define CFGR_MCO2_VALUE(reg)            FIELD_VALUE((reg), CFGR_MCO2_MASK, 30)
/*------------------Clock Interrupt Register----------------------------------*/
#define CIR_LSIRDYF                     BIT(0)
#define CIR_LSERDYF                     BIT(1)
#define CIR_HSIRDYF                     BIT(2)
#define CIR_HSERDYF                     BIT(3)
#define CIR_PLLRDYF                     BIT(4)
#define CIR_PLLI2SRDYF                  BIT(5)
#define CIR_CSSF                        BIT(7)
#define CIR_LSIRDYIE                    BIT(8)
#define CIR_LSERDYIE                    BIT(9)
#define CIR_HSIRDYIE                    BIT(10)
#define CIR_HSERDYIE                    BIT(11)
#define CIR_PLLRDYIE                    BIT(12)
#define CIR_PLLI2SRDYIE                 BIT(13)
#define CIR_LSIRDYC                     BIT(16)
#define CIR_LSERDYC                     BIT(17)
#define CIR_HSIRDYC                     BIT(18)
#define CIR_HSERDYC                     BIT(19)
#define CIR_PLLRDYC                     BIT(20)
#define CIR_PLLI2SRDYC                  BIT(21)
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
#define CSR_BORRSTF                     BIT(25)
#define CSR_PINRSTF                     BIT(26)
#define CSR_PORRSTF                     BIT(27)
#define CSR_SFTRSTF                     BIT(28)
#define CSR_IWDGRSTF                    BIT(29)
#define CSR_WWDGRSTF                    BIT(30)
#define CSR_LPWRRSTF                    BIT(31)
/*------------------Spread Spectrum Clock Generation Register-----------------*/
#define SSCGR_MODPER_MASK               BIT_FIELD(MASK(13), 0)
#define SSCGR_MODPER(value)             BIT_FIELD((value), 0)
#define SSCGR_MODPER_VALUE(reg) \
    FIELD_VALUE((reg), SSCGR_MODPER_MASK, 0)

#define SSCGR_INCSTEP_MASK              BIT_FIELD(MASK(15), 13)
#define SSCGR_INCSTEP(value)            BIT_FIELD((value), 13)
#define SSCGR_INCSTEP_VALUE(reg) \
    FIELD_VALUE((reg), SSCGR_INCSTEP_MASK, 13)

#define SSCGR_SPREADSEL                 BIT(30)
#define SSCGR_SSCGEN                    BIT(31)
/*------------------PLL I2S Configuration Register----------------------------*/
#define PLLI2SCFGR_PLLI2SN_MASK         BIT_FIELD(MASK(9), 6)
#define PLLI2SCFGR_PLLI2SN(value)       BIT_FIELD((value), 6)
#define PLLI2SCFGR_PLLI2SN_VALUE(reg) \
    FIELD_VALUE((reg), PLLI2SCFGR_PLLI2SN_MASK, 6)

#define PLLI2SCFGR_PLLI2SR_MASK         BIT_FIELD(MASK(3), 28)
#define PLLI2SCFGR_PLLI2SR(value)       BIT_FIELD((value), 28)
#define PLLI2SCFGR_PLLI2SR_VALUE(reg) \
    FIELD_VALUE((reg), PLLI2SCFGR_PLLI2SR_MASK, 28)
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_STM32_STM32F4XX_CLOCKING_DEFS_H_ */
