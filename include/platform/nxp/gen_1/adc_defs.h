/*
 * platform/nxp/gen_1/adc_defs.h
 * Copyright (C) 2012 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef HALM_PLATFORM_NXP_GEN_1_ADC_DEFS_H_
#define HALM_PLATFORM_NXP_GEN_1_ADC_DEFS_H_
/*----------------------------------------------------------------------------*/
#include <bits.h>
/*------------------Control Register------------------------------------------*/
#define CR_SEL_MASK                     BIT_FIELD(MASK(8), 0)
#define CR_SEL(value)                   BIT_FIELD((value), 0)
#define CR_SEL_CHANNEL(channel)         BIT_FIELD(BIT(channel), 0)
#define CR_SEL_VALUE(reg)               FIELD_VALUE((reg), CR_SEL_MASK, 0)
#define CR_CLKDIV_MASK                  BIT_FIELD(MASK(8), 8)
#define CR_CLKDIV(value)                BIT_FIELD((value), 8)
#define CR_CLKDIV_VALUE(reg)            FIELD_VALUE((reg), CR_CLKDIV_MASK, 8)
#define CR_BURST                        BIT(16)
#define CR_START_MASK                   BIT_FIELD(MASK(3), 24)
#define CR_START(value)                 BIT_FIELD((value), 24)
#define CR_START_VALUE(reg)             FIELD_VALUE((reg), CR_START_MASK, 24)
#define CR_EDGE                         BIT(27)
/* Device-specific bits */
#define CR_CLKS_MASK                    BIT_FIELD(MASK(2), 17)
#define CR_CLKS(value)                  BIT_FIELD((value), 17)
#define CR_CLKS_VALUE(reg)              FIELD_VALUE((reg), CR_CLKS_MASK, 17)
#define CR_PDN                          BIT(21)
/*------------------Global Data Register--------------------------------------*/
#define GDR_RESULT_MASK                 BIT_FIELD(MASK(16), 0)
#define GDR_RESULT_VALUE(reg)           FIELD_VALUE((reg), GDR_RESULT_MASK, 0)
#define GDR_CHN_MASK                    BIT_FIELD(MASK(3), 24)
#define GDR_CHN_VALUE(reg)              FIELD_VALUE((reg), GDR_CHN_MASK, 24)
#define GDR_OVERRUN                     BIT(30)
#define GDR_DONE                        BIT(31)
/*------------------Interrupt Enable register---------------------------------*/
#define INTEN_AD(channel)               BIT(channel)
#define INTEN_ADG                       BIT(8)
/*------------------Data Registers--------------------------------------------*/
#define DR_RESULT_MASK                  BIT_FIELD(MASK(16), 0)
#define DR_RESULT_VALUE(reg)            FIELD_VALUE((reg), DR_RESULT_MASK, 0)
#define DR_OVERRUN                      BIT(30)
#define DR_DONE                         BIT(31)
/*------------------Status register-------------------------------------------*/
#define STAT_DONE(channel)              BIT(channel)
#define STAT_OVERRUN(channel)           BIT((channel) + 8)
#define STAT_ADINT                      BIT(16)
/*------------------Trim register---------------------------------------------*/
#define TRIM_ADCOFFS(value)             BIT_FIELD((value), 4)
#define TRIM_ADCOFFS_MASK               BIT_FIELD(MASK(4), 4)
#define TRIM_ADCOFFS_VALUE(reg)         FIELD_VALUE((reg), TRIM_ADCOFFS_MASK, 4)
#define TRIM_VALUE_MASK                 BIT_FIELD(MASK(4), 8)
#define TRIM_VALUE(reg)                 FIELD_VALUE((reg), TRIM_VALUE_MASK, 8)
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_NXP_GEN_1_ADC_DEFS_H_ */
