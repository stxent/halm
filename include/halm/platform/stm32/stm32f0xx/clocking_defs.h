/*
 * halm/platform/stm32/stm32f0xx/clocking_defs.h
 * Copyright (C) 2020 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_STM32_STM32F0XX_CLOCKING_DEFS_H_
#define HALM_PLATFORM_STM32_STM32F0XX_CLOCKING_DEFS_H_
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
/*------------------Clock Control Register 2----------------------------------*/
#define CR2_HSI14ON                     BIT(0)
#define CR2_HSI14RDY                    BIT(1)
#define CR2_HSI14DIS                    BIT(2)

#define CR2_HSI14TRIM_MASK              BIT_FIELD(MASK(5), 3)
#define CR2_HSI14TRIM(value)            BIT_FIELD((value), 3)
#define CR2_HSI14TRIM_VALUE(reg) \
    FIELD_VALUE((reg), CR2_HSI14TRIM_MASK, 3)

#define CR2_HSI14CAL_MASK               BIT_FIELD(MASK(8), 8)
#define CR2_HSI14CAL(value)             BIT_FIELD((value), 8)
#define CR2_HSI14CAL_VALUE(reg) \
    FIELD_VALUE((reg), CR2_HSI14CAL_MASK, 8)

#define CR2_HSI48ON                     BIT(16)
#define CR2_HSI48RDY                    BIT(17)

#define CR2_HSI48CAL_MASK               BIT_FIELD(MASK(8), 24)
#define CR2_HSI48CAL(value)             BIT_FIELD((value), 24)
#define CR2_HSI48CAL_VALUE(reg) \
    FIELD_VALUE((reg), CR2_HSI48CAL_MASK, 24)
/*------------------Clock Configuration Register------------------------------*/
enum
{
  CFGR_SW_HSI   = 0,
  CFGR_SW_HSE   = 1,
  CFGR_SW_PLL   = 2,
  CFGR_SW_HSI48 = 3
};

enum
{
  CFGR_PLLSRC_HSI_DIV2 = 0,
  CFGR_PLLSRC_HSI      = 1,
  CFGR_PLLSRC_HSE      = 2,
  CFGR_PLLSRC_HSI48    = 3
};

enum
{
  CFGR_MCO_NONE   = 0x00,
  CFGR_MCO_HSI14  = 0x01,
  CFGR_MCO_LSI    = 0x02,
  CFGR_MCO_LSE    = 0x03,
  CFGR_MCO_SYSCLK = 0x04,
  CFGR_MCO_HSI    = 0x05,
  CFGR_MCO_HSE    = 0x06,
  CFGR_MCO_PLL    = 0x07,
  CFGR_MCO_HSI48  = 0x08
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

#define CFGR_PPRE_MASK                  BIT_FIELD(MASK(3), 8)
#define CFGR_PPRE(value)                BIT_FIELD((value), 8)
#define CFGR_PPRE_VALUE(reg)            FIELD_VALUE((reg), CFGR_PPRE_MASK, 8)

/* Obsolete */
#define CFGR_ADCPRE                     BIT(14)

#define CFGR_PLLSRC_MASK                BIT_FIELD(MASK(2), 15)
#define CFGR_PLLSRC(value)              BIT_FIELD((value), 15)
#define CFGR_PLLSRC_VALUE(reg)          FIELD_VALUE((reg), CFGR_PLLSRC_MASK, 15)

#define CFGR_PLLXTPRE                   BIT(17)

#define CFGR_PLLMUL_MASK                BIT_FIELD(MASK(4), 18)
#define CFGR_PLLMUL(value)              BIT_FIELD((value), 18)
#define CFGR_PLLMUL_VALUE(reg)          FIELD_VALUE((reg), CFGR_PLLMUL_MASK, 18)

#define CFGR_MCO_MASK                   BIT_FIELD(MASK(4), 24)
#define CFGR_MCO(value)                 BIT_FIELD((value), 24)
#define CFGR_MCO_VALUE(reg)             FIELD_VALUE((reg), CFGR_MCO_MASK, 24)

#define CFGR_MCOPRE_MASK                BIT_FIELD(MASK(3), 28)
#define CFGR_MCOPRE(value)              BIT_FIELD((value), 28)
#define CFGR_MCOPRE_VALUE(reg)          FIELD_VALUE((reg), CFGR_MCOPRE_MASK, 28)

#define CFGR_PLLNODIV                   BIT(31)
/*------------------Clock Interrupt Register----------------------------------*/
#define CIR_LSIRDYF                     BIT(0)
#define CIR_LSERDYF                     BIT(1)
#define CIR_HSIRDYF                     BIT(2)
#define CIR_HSERDYF                     BIT(3)
#define CIR_PLLRDYF                     BIT(4)
#define CIR_HSI14RDYF                   BIT(5)
#define CIR_HSI48RDYF                   BIT(6)
#define CIR_CSSF                        BIT(7)
#define CIR_LSIRDYIE                    BIT(8)
#define CIR_LSERDYIE                    BIT(9)
#define CIR_HSIRDYIE                    BIT(10)
#define CIR_HSERDYIE                    BIT(11)
#define CIR_PLLRDYIE                    BIT(12)
#define CIR_HSI14RDYIE                  BIT(13)
#define CIR_HSI48RDYIE                  BIT(14)
#define CIR_LSIRDYC                     BIT(16)
#define CIR_LSERDYC                     BIT(17)
#define CIR_HSIRDYC                     BIT(18)
#define CIR_HSERDYC                     BIT(19)
#define CIR_PLLRDYC                     BIT(20)
#define CIR_HSI14RDYC                   BIT(21)
#define CIR_HSI48RDYC                   BIT(22)
#define CIR_CSSC                        BIT(23)
/*------------------RTC Domain Control Register-------------------------------*/
#define BDCR_LSEON                      BIT(0)
#define BDCR_LSERDY                     BIT(1)
#define BDCR_LSEBYP                     BIT(2)

#define BDCR_LSEDRV_MASK                BIT_FIELD(MASK(2), 3)
#define BDCR_LSEDRV(value)              BIT_FIELD((value), 3)
#define BDCR_LSEDRV_VALUE(reg)          FIELD_VALUE((reg), BDCR_LSEDRV_MASK, 3)

#define BDCR_RTCSEL_MASK                BIT_FIELD(MASK(2), 8)
#define BDCR_RTCSEL(value)              BIT_FIELD((value), 8)
#define BDCR_RTCSEL_VALUE(reg)          FIELD_VALUE((reg), BDCR_RTCSEL_MASK, 8)

#define BDCR_RTCEN                      BIT(15)
#define BDCR_BDRST                      BIT(16)
/*------------------Control/Status Register-----------------------------------*/
#define CSR_LSION                       BIT(0)
#define CSR_LSIRDY                      BIT(1)
#define CSR_V18PWRRSTF                  BIT(23)
#define CSR_RMVF                        BIT(24)
#define CSR_OBLRSTF                     BIT(25)
#define CSR_PINRSTF                     BIT(26)
#define CSR_PORRSTF                     BIT(27)
#define CSR_SFTRSTF                     BIT(28)
#define CSR_IWDGRSTF                    BIT(29)
#define CSR_WWDGRSTF                    BIT(30)
#define CSR_LPWRRSTF                    BIT(31)
/*------------------Clock Configuration Register 2----------------------------*/
#define CFGR2_PREDIV_MASK               BIT_FIELD(MASK(4), 0)
#define CFGR2_PREDIV(value)             BIT_FIELD((value), 0)
#define CFGR2_PREDIV_VALUE(reg) \
    FIELD_VALUE((reg), CFGR2_PREDIV_MASK, 0)
/*------------------Clock Configuration Register 3----------------------------*/
#define CFGR3_USART1SW_MASK             BIT_FIELD(MASK(2), 0)
#define CFGR3_USART1SW(value)           BIT_FIELD((value), 0)
#define CFGR3_USART1SW_VALUE(reg) \
    FIELD_VALUE((reg), CFGR3_USART1SW_MASK, 0)

#define CFGR3_I2C1SW                    BIT(4)
#define CFGR3_CECSW                     BIT(6)
#define CFGR3_USBSW                     BIT(7)

/* Obsolete */
#define CFGR3_ADCSW                     BIT(8)

#define CFGR3_USART2SW_MASK             BIT_FIELD(MASK(2), 16)
#define CFGR3_USART2SW(value)           BIT_FIELD((value), 16)
#define CFGR3_USART2SW_VALUE(reg) \
    FIELD_VALUE((reg), CFGR3_USART2SW_MASK, 16)

#define CFGR3_USART3SW_MASK             BIT_FIELD(MASK(2), 18)
#define CFGR3_USART3SW(value)           BIT_FIELD((value), 18)
#define CFGR3_USART3SW_VALUE(reg) \
    FIELD_VALUE((reg), CFGR3_USART3SW_MASK, 18)
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_STM32_STM32F0XX_CLOCKING_DEFS_H_ */
