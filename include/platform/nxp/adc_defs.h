/*
 * platform/nxp/adc_defs.h
 * Copyright (C) 2012 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef ADC_DEFS_H_
#define ADC_DEFS_H_
/*----------------------------------------------------------------------------*/
#include <macro.h>
/*------------------Control Register------------------------------------------*/
#define CR_SEL_MASK                     BIT_FIELD(0xFF, 0)
#define CR_SEL(channel)                 BIT_FIELD((channel), 0)
#define CR_SEL_VALUE(reg)               ((reg) & 0xFF)
#define CR_CLKDIV_MASK                  BIT_FIELD(0xFF, 8)
#define CR_CLKDIV(value)                BIT_FIELD((value), 8)
#define CR_CLKDIV_VALUE(reg)            (((reg) >> 8) & 0xFF)
#define CR_BURST                        BIT(16)
#define CR_PDN                          BIT(21)
#define CR_START_MASK                   BIT_FIELD(0x07, 24)
#define CR_START(value)                 BIT_FIELD((value), 24)
#define CR_START_VALUE(reg)             (((reg) >> 24) & 0x07)
#define CR_EDGE                         BIT(27)
/*------------------Global Data Register--------------------------------------*/
#define GDR_RESULT_VALUE(reg, width) \
		(((reg) >> (16 - (width))) & ((1 << (width)) - 1))
#define GDR_CHN_VALUE(reg)              (((reg) >> 24) & 0x07)
#define GDR_OVERRUN                     BIT(30)
#define GDR_DONE                        BIT(31)
/*------------------Interrupt Enable register---------------------------------*/
#define INTEN_AD(channel)               BIT((channel))
#define INTEN_ADG                       BIT(8)
/*------------------Data Registers--------------------------------------------*/
#define DR_RESULT_VALUE(reg, width) \
    (((reg) >> (16 - (width))) & ((1 << (width)) - 1))
#define DR_OVERRUN                      BIT(30)
#define DR_DONE                         BIT(31)
/*------------------Status register-------------------------------------------*/
#define STAT_DONE(channel)              BIT((channel))
#define STAT_OVERRUN(channel)           BIT((channel) + 8)
#define STAT_ADINT                      BIT(16)
/*------------------Trim register---------------------------------------------*/
#define TRIM_ADCOFFS(value)             BIT_FIELD((value), 4)
#define TRIM_ADCOFFS_VALUE(reg)         (((reg) >> 4) & 0x0F)
#define TRIM_VALUE(reg)                 (((reg) >> 8) & 0x0F)
/*----------------------------------------------------------------------------*/
#endif /* ADC_DEFS_H_ */
