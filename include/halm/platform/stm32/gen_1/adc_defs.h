/*
 * halm/platform/stm32/gen_1/adc_defs.h
 * Copyright (C) 2023 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_STM32_GEN_1_ADC_DEFS_H_
#define HALM_PLATFORM_STM32_GEN_1_ADC_DEFS_H_
/*----------------------------------------------------------------------------*/
#include <xcore/bits.h>
/*------------------Status Register-------------------------------------------*/
#define SR_AWD                          BIT(0)
#define SR_EOC                          BIT(1)
#define SR_JEOC                         BIT(2)
#define SR_JSTRT                        BIT(3)
#define SR_STRT                         BIT(4)
#define SR_OVR                          BIT(5)

#define SR_MASK                         BIT_FIELD(MASK(6), 0)
/*------------------Control Register 1----------------------------------------*/
enum
{
  /* 12 bit, 15 ADCCLK cycles */
  RES_12BIT = 0,
  /* 10 bit, 13 ADCCLK cycles */
  RES_10BIT = 1,
  /* 8 bit, 11 ADCCLK cycles */
  RES_8BIT  = 2,
  /* 6 bit, 9 ADCCLK cycles */
  RES_6BIT  = 3
};

#define CR1_AWDCH_MASK                  BIT_FIELD(MASK(5), 0)
#define CR1_AWDCH(value)                BIT_FIELD((value), 0)
#define CR1_AWDCH_VALUE(reg)            FIELD_VALUE((reg), CR1_AWDCH_MASK, 0)

#define CR1_EOCIE                       BIT(5)
#define CR1_AWDIE                       BIT(6)
#define CR1_JEOCIE                      BIT(7)
#define CR1_SCAN                        BIT(8)
#define CR1_AWDSGL                      BIT(9)
#define CR1_JAUTO                       BIT(10)
#define CR1_DISCEN                      BIT(11)
#define CR1_JDISCEN                     BIT(12)

#define CR1_DISCNUM_MASK                BIT_FIELD(MASK(3), 13)
#define CR1_DISCNUM(value)              BIT_FIELD((value), 13)
#define CR1_DISCNUM_VALUE(reg)          FIELD_VALUE((reg), CR1_DISCNUM_MASK, 13)

#define CR1_JAWDEN                      BIT(22)
#define CR1_AWDEN                       BIT(23)

#define CR1_RES_MASK                    BIT_FIELD(MASK(2), 24)
#define CR1_RES(value)                  BIT_FIELD((value), 24)
#define CR1_RES_VALUE(reg)              FIELD_VALUE((reg), CR1_RES_MASK, 24)

#define CR1_OVRIE                       BIT(26)
/*------------------Control Register 2----------------------------------------*/
#define CR2_ADON                        BIT(0)
#define CR2_CONT                        BIT(1)
#define CR2_DMA                         BIT(8)
#define CR2_EOCS                        BIT(10)
#define CR2_ALIGN                       BIT(11)
#define CR2_JSWSTART                    BIT(22)
#define CR2_SWSTART                     BIT(30)

#ifdef CONFIG_PLATFORM_STM32_ADC_BASIC

#  define CR2_CAL                       BIT(2)
#  define CR2_RSTCAL                    BIT(3)
#  define CR2_DDS                       0
#  define CR2_JEXTTRIG                  BIT(15)
#  define CR2_EXTTRIG                   BIT(20)

#  define CR2_JEXTSEL_MASK              BIT_FIELD(MASK(3), 12)
#  define CR2_JEXTSEL(value)            BIT_FIELD((value), 12)
#  define CR2_JEXTSEL_VALUE(reg)        FIELD_VALUE((reg), CR2_JEXTSEL_MASK, 12)

#  define CR2_JEXTEN_MASK               0
#  define CR2_JEXTEN(value)             ((value) & 0)
#  define CR2_JEXTEN_VALUE(reg)         ((reg) & 0)

#  define CR2_EXTSEL_MASK               BIT_FIELD(MASK(3), 17)
#  define CR2_EXTSEL(value)             BIT_FIELD((value), 17)
#  define CR2_EXTSEL_VALUE(reg)         FIELD_VALUE((reg), CR2_EXTSEL_MASK, 17)

#  define CR2_EXTEN_MASK                0
#  define CR2_EXTEN(value)              ((value) & 0)
#  define CR2_EXTEN_VALUE(reg)          ((reg) & 0)

#else /* CONFIG_PLATFORM_STM32_ADC_BASIC */

enum
{
  EXTEN_DISABLED  = 0,
  EXTEN_RISING    = 1,
  EXTEN_FALLING   = 2,
  EXTEN_TOGGLE    = 3
};

#  define CR2_DDS                       BIT(9)
#  define CR2_JEXTTRIG                  0
#  define CR2_EXTTRIG                   0

#  define CR2_JEXTSEL_MASK              BIT_FIELD(MASK(4), 16)
#  define CR2_JEXTSEL(value)            BIT_FIELD((value), 16)
#  define CR2_JEXTSEL_VALUE(reg)        FIELD_VALUE((reg), CR2_JEXTSEL_MASK, 16)

#  define CR2_JEXTEN_MASK               BIT_FIELD(MASK(2), 20)
#  define CR2_JEXTEN(value)             BIT_FIELD((value), 20)
#  define CR2_JEXTEN_VALUE(reg)         FIELD_VALUE((reg), CR2_JEXTEN_MASK, 20)

#  define CR2_EXTSEL_MASK               BIT_FIELD(MASK(4), 24)
#  define CR2_EXTSEL(value)             BIT_FIELD((value), 24)
#  define CR2_EXTSEL_VALUE(reg)         FIELD_VALUE((reg), CR2_EXTSEL_MASK, 24)

#  define CR2_EXTEN_MASK                BIT_FIELD(MASK(2), 28)
#  define CR2_EXTEN(value)              BIT_FIELD((value), 28)
#  define CR2_EXTEN_VALUE(reg)          FIELD_VALUE((reg), CR2_EXTEN_MASK, 28)

#endif /* CONFIG_PLATFORM_STM32_ADC_BASIC */
/*------------------Sample Time Registers-------------------------------------*/
#define SMPR_SMP_MASK(offset)           BIT_FIELD(MASK(3), (offset) * 3)
#define SMPR_SMP(offset, value)         BIT_FIELD((value), (offset) * 3)
#define SMPR_SMP_VALUE(offset, reg) \
    FIELD_VALUE((reg), SMPR_SMP_MASK(offset), (offset) * 3)

#define SMPR_SMP_REV_INDEX(in)          ((in) / 10)
#define SMPR_SMP_OFFSET(in)             ((in) % 10)
/*------------------Regular Sequence Registers--------------------------------*/
#define SQR1_L_MASK                     BIT_FIELD(MASK(4), 20)
#define SQR1_L(value)                   BIT_FIELD((value), 20)
#define SQR1_L_VALUE(reg)               FIELD_VALUE((reg), SQR1_L_MASK, 20)

#define SQR_SQ_MASK(offset)             BIT_FIELD(MASK(5), (offset) * 5)
#define SQR_SQ(offset, value)           BIT_FIELD((value), (offset) * 5)
#define SQR_SQ_VALUE(offset, reg) \
    FIELD_VALUE((reg), SQR_SQ_MASK(offset), (offset) * 5)

#define SQR_SQ_REV_INDEX(in)            ((in) / 6)
#define SQR_SQ_OFFSET(in)               ((in) % 6)
/*------------------Injected Sequence Register--------------------------------*/
#define JSQR_JSQ_MASK(sequence)         BIT_FIELD(MASK(5), (sequence) * 5)
#define JSQR_JSQ(sequence, value)       BIT_FIELD((value), (sequence) * 5)
#define JSQR_JSQ_VALUE(sequence, reg) \
    FIELD_VALUE((reg), JSQR_JSQ_MASK(sequence), (sequence) * 5)

#define JSQR_JL_MASK                    BIT_FIELD(MASK(2), 20)
#define JSQR_JL(value)                  BIT_FIELD((value), 20)
#define JSQR_JL_VALUE(reg)              FIELD_VALUE((reg), JSQR_JL_MASK, 20)
/*------------------Common Status Register------------------------------------*/
#define CSR_AWD(channel)                BIT((channel) * 8)
#define CSR_EOC(channel)                BIT((channel) * 8 + 1)
#define CSR_JEOC(channel)               BIT((channel) * 8 + 2)
#define CSR_JSTRT(channel)              BIT((channel) * 8 + 3)
#define CSR_STRT(channel)               BIT((channel) * 8 + 4)
#define CSR_OVR(channel)                BIT((channel) * 8 + 5)

#define CSR_MASK(channel)               BIT_FIELD(MASK(6), (channel) * 8)
/*------------------Common Control Register-----------------------------------*/
#define CCR_MULTI_MASK                  BIT_FIELD(MASK(5), 0)
#define CCR_MULTI(value)                BIT_FIELD((value), 0)
#define CCR_MULTI_VALUE(reg)            FIELD_VALUE((reg), CCR_MULTI_MASK, 0)

#define CCR_DELAY_MASK                  BIT_FIELD(MASK(4), 8)
#define CCR_DELAY(value)                BIT_FIELD((value), 8)
#define CCR_DELAY_VALUE(reg)            FIELD_VALUE((reg), CCR_DELAY_MASK, 8)

#define CCR_DDS                         BIT(13)

#define CCR_DMA_MASK                    BIT_FIELD(MASK(2), 14)
#define CCR_DMA(value)                  BIT_FIELD((value), 14)
#define CCR_DMA_VALUE(reg)              FIELD_VALUE((reg), CCR_DMA_MASK, 14)

#define CCR_ADCPRE_MASK                 BIT_FIELD(MASK(2), 16)
#define CCR_ADCPRE(value)               BIT_FIELD((value), 16)
#define CCR_ADCPRE_VALUE(reg)           FIELD_VALUE((reg), CCR_ADCPRE_MASK, 16)

#define CCR_VBATE                       BIT(22)
#define CCR_TSVREFE                     BIT(23)
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_STM32_GEN_1_ADC_DEFS_H_ */
