/*
 * halm/platform/stm32/gen_2/adc_defs.h
 * Copyright (C) 2023 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_STM32_GEN_2_ADC_DEFS_H_
#define HALM_PLATFORM_STM32_GEN_2_ADC_DEFS_H_
/*----------------------------------------------------------------------------*/
#include <xcore/bits.h>
/*------------------Interrupt and Status Register-----------------------------*/
#define ISR_ADRDY                       BIT(0)
#define ISR_EOSMP                       BIT(1)
#define ISR_EOC                         BIT(2)
#define ISR_EOSEQ                       BIT(3)
#define ISR_OVR                         BIT(4)
#define ISR_AWD                         BIT(7)

#define ISR_MASK \
    (ISR_ADRDY | ISR_EOSMP | ISR_EOC | ISR_EOSEQ | ISR_OVR | ISR_AWD)
/*------------------Interrupt Enable Register---------------------------------*/
#define IER_ADRDYIE                     BIT(0)
#define IER_EOSMPIE                     BIT(1)
#define IER_EOCIE                       BIT(2)
#define IER_EOSEQIE                     BIT(3)
#define IER_OVRIE                       BIT(4)
#define IER_AWDIE                       BIT(7)
/*------------------Control Register------------------------------------------*/
#define CR_ADEN                         BIT(0)
#define CR_ADDIS                        BIT(1)
#define CR_ADSTART                      BIT(2)
#define CR_ADSTP                        BIT(4)
#define CR_ADCAL                        BIT(31)
/*------------------Configuration Register 1----------------------------------*/
enum
{
  EXTEN_DISABLED  = 0,
  EXTEN_RISING    = 1,
  EXTEN_FALLING   = 2,
  EXTEN_TOGGLE    = 3
};

enum
{
  /* 12 bit, 14 ADCCLK cycles */
  RES_12BIT = 0,
  /* 10 bit, 13 ADCCLK cycles */
  RES_10BIT = 1,
  /* 8 bit, 11 ADCCLK cycles */
  RES_8BIT  = 2,
  /* 6 bit, 9 ADCCLK cycles */
  RES_6BIT  = 3
};

#define CFGR1_DMAEN                     BIT(0)
#define CFGR1_DMACFG                    BIT(1)
#define CFGR1_SCANDIR                   BIT(2)

#define CFGR1_RES_MASK                  BIT_FIELD(MASK(2), 3)
#define CFGR1_RES(value)                BIT_FIELD((value), 3)
#define CFGR1_RES_VALUE(reg)            FIELD_VALUE((reg), CFGR1_RES_MASK, 3)

#define CFGR1_ALIGN                     BIT(5)

#define CFGR1_EXTSEL_MASK               BIT_FIELD(MASK(4), 6)
#define CFGR1_EXTSEL(value)             BIT_FIELD((value), 6)
#define CFGR1_EXTSEL_VALUE(reg)         FIELD_VALUE((reg), CFGR1_EXTSEL_MASK, 6)

#define CFGR1_EXTEN_MASK                BIT_FIELD(MASK(2), 10)
#define CFGR1_EXTEN(value)              BIT_FIELD((value), 10)
#define CFGR1_EXTEN_VALUE(reg)          FIELD_VALUE((reg), CFGR1_EXTEN_MASK, 10)

#define CFGR1_OVRMOD                    BIT(12)
#define CFGR1_CONT                      BIT(13)
#define CFGR1_WAIT                      BIT(14)
#define CFGR1_AUTOFF                    BIT(15)
#define CFGR1_DISCEN                    BIT(16)

#define CFGR1_AWDSGL                    BIT(22)
#define CFGR1_AWDEN                     BIT(23)

#define CFGR1_AWDCH_MASK                BIT_FIELD(MASK(5), 26)
#define CFGR1_AWDCH(value)              BIT_FIELD((value), 26)
#define CFGR1_AWDCH_VALUE(reg)          FIELD_VALUE((reg), CFGR1_AWDCH_MASK, 26)
/*------------------Sampling time register------------------------------------*/
#define SMPR_SMP_MASK                   BIT_FIELD(MASK(3), 0)
#define SMPR_SMP(value)                 BIT_FIELD((value), 0)
#define SMPR_SMP_VALUE(reg)             FIELD_VALUE((reg), SMPR_SMP_MASK, 0)
/*------------------Watchdog Threshold register-------------------------------*/
#define TR_LT_MASK                      BIT_FIELD(MASK(12), 0)
#define TR_LT(value)                    BIT_FIELD((value), 0)
#define TR_LT_VALUE(reg)                FIELD_VALUE((reg), TR_LT_MASK, 0)

#define TR_HT_MASK                      BIT_FIELD(MASK(12), 16)
#define TR_HT(value)                    BIT_FIELD((value), 16)
#define TR_HT_VALUE(reg)                FIELD_VALUE((reg), TR_HT_MASK, 16)
/*------------------Channel Selection Register--------------------------------*/
#define CHSELR_CHSEL(channel)           BIT(channel)
#define CHSELR_CHSEL_MASK               MASK(19)
/*------------------Common Configuration Register-----------------------------*/
#define CCR_VREFEN                      BIT(22)
#define CCR_TSEN                        BIT(23)
#define CCR_VBATEN                      BIT(24)
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_STM32_GEN_2_ADC_DEFS_H_ */
