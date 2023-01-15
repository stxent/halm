/*
 * halm/platform/numicro/adc_defs.h
 * Copyright (C) 2023 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_NUMICRO_ADC_DEFS_H_
#define HALM_PLATFORM_NUMICRO_ADC_DEFS_H_
/*----------------------------------------------------------------------------*/
#include <xcore/bits.h>
/*------------------Data Registers--------------------------------------------*/
#define ADDR_RSLT(value)                BIT_FIELD((value), 0)
#define ADDR_RSLT_MASK                  BIT_FIELD(MASK(16), 0)
#define ADDR_RSLT_VALUE(reg)            FIELD_VALUE((reg), ADDR_RSLT_MASK, 0)

#define ADDR_OVERRUN                    BIT(16)
#define ADDR_VALID                      BIT(17)
/*------------------Control Registers-----------------------------------------*/
enum
{
  ADMD_SINGLE           = 0,
  ADMD_BURST            = 1,
  ADMD_SINGLE_SCAN      = 2,
  ADMD_CONTINUOUS_SCAN  = 3
};

enum
{
  TRGS_EXTERNAL = 0,
  TRGS_TIMER    = 1,
  TRGS_BPWM     = 2,
  TRGS_PWM      = 3
};

enum
{
  TRGCOND_LOW     = 0,
  TRGCOND_HIGH    = 1,
  TRGCOND_FALLING = 2,
  TRGCOND_RISING  = 3
};

#define ADCR_ADEN                       BIT(0)
#define ADCR_ADIE                       BIT(1)

#define ADCR_ADMD(value)                BIT_FIELD((value), 2)
#define ADCR_ADMD_MASK                  BIT_FIELD(MASK(2), 2)
#define ADCR_ADMD_VALUE(reg)            FIELD_VALUE((reg), ADCR_ADMD_MASK, 2)

#define ADCR_TRGS(value)                BIT_FIELD((value), 4)
#define ADCR_TRGS_MASK                  BIT_FIELD(MASK(2), 4)
#define ADCR_TRGS_VALUE(reg)            FIELD_VALUE((reg), ADCR_TRGS_MASK, 4)

#define ADCR_TRGCOND(value)             BIT_FIELD((value), 6)
#define ADCR_TRGCOND_MASK               BIT_FIELD(MASK(2), 6)
#define ADCR_TRGCOND_VALUE(reg)         FIELD_VALUE((reg), ADCR_TRGCOND_MASK, 6)

#define ADCR_TRGEN                      BIT(8)
#define ADCR_PTEN                       BIT(9)
#define ADCR_DIFFEN                     BIT(10)
#define ADCR_ADST                       BIT(11)
#define ADCR_RESET                      BIT(12)
#define ADCR_DMOF                       BIT(31)
/*------------------Compare Registers-----------------------------------------*/
#define ADCMPR_CMPEN                    BIT(0)
#define ADCMPR_CMPIE                    BIT(1)
#define ADCMPR_CMPCOND                  BIT(2)

#define ADCMPR_CMPCH(value)             BIT_FIELD((value), 3)
#define ADCMPR_CMPCH_MASK               BIT_FIELD(MASK(5), 3)
#define ADCMPR_CMPCH_VALUE(reg)         FIELD_VALUE((reg), ADCMPR_CMPCH_MASK, 3)

#define ADCMPR_CMPMATCNT(value)         BIT_FIELD((value), 8)
#define ADCMPR_CMPMATCNT_MASK           BIT_FIELD(MASK(4), 8)
#define ADCMPR_CMPMATCNT_VALUE(reg) \
    FIELD_VALUE((reg), ADCMPR_CMPMATCNT_MASK, 8)

#define ADCMPR_CMPWEN                   BIT(15)

#define ADCMPR_CMPD(value)              BIT_FIELD((value), 16)
#define ADCMPR_CMPD_MASK                BIT_FIELD(MASK(12), 16)
#define ADCMPR_CMPD_VALUE(reg)          FIELD_VALUE((reg), ADCMPR_CMPD_MASK, 16)
/*------------------Status Register 0-----------------------------------------*/
#define ADSR0_ADF                       BIT(0)
#define ADSR0_CMPF0                     BIT(1)
#define ADSR0_CMPF1                     BIT(2)
#define ADSR0_BUSY                      BIT(7)
#define ADSR0_VALIDF                    BIT(8)
#define ADSR0_OVERRUNF                  BIT(16)

#define ADSR0_CHANNEL(value)            BIT_FIELD((value), 27)
#define ADSR0_CHANNEL_MASK              BIT_FIELD(MASK(5), 27)
#define ADSR0_CHANNEL_VALUE(reg) \
    FIELD_VALUE((reg), ADSR0_CHANNEL_MASK, 27)
/*------------------Channel Floating Detect Control register------------------*/
#define CFDCTL_PRECHEN                  BIT(0)
#define CFDCTL_DISCHEN                  BIT(1)
#define CFDCTL_FDETCHEN                 BIT(8)
/*------------------Calibration Mode Register---------------------------------*/
#define ADCALR_CALEN                    BIT(0)
#define ADCALR_CALIE                    BIT(1)
/*------------------Calibration Status Register-------------------------------*/
#define ADCALSTSR_CALIF                 BIT(0)
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_NUMICRO_ADC_DEFS_H_ */
