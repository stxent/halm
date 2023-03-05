/*
 * halm/platform/numicro/eadc_defs.h
 * Copyright (C) 2023 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_NUMICRO_EADC_DEFS_H_
#define HALM_PLATFORM_NUMICRO_EADC_DEFS_H_
/*----------------------------------------------------------------------------*/
#include <xcore/bits.h>
/*------------------Data Registers--------------------------------------------*/
#define DAT_RESULT(value)               BIT_FIELD((value), 0)
#define DAT_RESULT_MASK                 BIT_FIELD(MASK(16), 0)
#define DAT_RESULT_VALUE(reg)           FIELD_VALUE((reg), DAT_RESULT_MASK, 0)

#define DAT_OVERRUN                     BIT(16)
#define DAT_VALID                       BIT(17)
/*------------------Control Registers-----------------------------------------*/
enum
{
  RESSEL_6BIT   = 0,
  RESSEL_8BIT   = 1,
  RESSEL_10BIT  = 2,
  RESSEL_12BIT  = 3
};

#define CTL_ADCEN                       BIT(0)
#define CTL_ADCRST                      BIT(1)
#define CTL_ADCIEN0                     BIT(2)
#define CTL_ADCIEN1                     BIT(3)
#define CTL_ADCIEN2                     BIT(4)
#define CTL_ADCIEN3                     BIT(5)

#define CTL_RESSEL(value)               BIT_FIELD((value), 6)
#define CTL_RESSEL_MASK                 BIT_FIELD(MASK(2), 6)
#define CTL_RESSEL_VALUE(reg)           FIELD_VALUE((reg), CTL_RESSEL_MASK, 6)

#define CTL_DIFFEN                      BIT(8)
#define CTL_DMOF                        BIT(9)
#define CTL_PDMAEN                      BIT(11)
/*------------------Sample Module 0..3 control registers----------------------*/
#define SCTL0_3_CHSEL(value)            BIT_FIELD((value), 0)
#define SCTL0_3_CHSEL_MASK              BIT_FIELD(MASK(4), 0)
#define SCTL0_3_CHSEL_VALUE(reg) \
    FIELD_VALUE((reg), SCTL0_3_CHSEL_MASK, 0)

#define SCTL0_3_EXTREN                  BIT(4)
#define SCTL0_3_EXTFEN                  BIT(5)

#define SCTL0_3_TRGDLYDIV(value)        BIT_FIELD((value), 6)
#define SCTL0_3_TRGDLYDIV_MASK          BIT_FIELD(MASK(2), 6)
#define SCTL0_3_TRGDLYDIV_VALUE(reg) \
    FIELD_VALUE((reg), SCTL0_3_TRGDLYDIV_MASK, 6)

#define SCTL0_3_TRGDLYCNT(value)        BIT_FIELD((value), 8)
#define SCTL0_3_TRGDLYCNT_MASK          BIT_FIELD(MASK(8), 8)
#define SCTL0_3_TRGDLYCNT_VALUE(reg) \
    FIELD_VALUE((reg), SCTL0_3_TRGDLYCNT_MASK, 8)

#define SCTL0_3_TRGSEL(value)           BIT_FIELD((value), 16)
#define SCTL0_3_TRGSEL_MASK             BIT_FIELD(MASK(5), 16)
#define SCTL0_3_TRGSEL_VALUE(reg) \
    FIELD_VALUE((reg), SCTL0_3_TRGSEL_MASK, 16)

#define SCTL0_3_INTPOS                  BIT(22)
#define SCTL0_3_DBMEN                   BIT(23)

#define SCTL0_3_EXTSMPT(value)          BIT_FIELD((value), 24)
#define SCTL0_3_EXTSMPT_MASK            BIT_FIELD(MASK(8), 24)
#define SCTL0_3_EXTSMPT_VALUE(reg) \
    FIELD_VALUE((reg), SCTL0_3_EXTSMPT_MASK, 24)
/*------------------Sample Module 4..15 control registers---------------------*/
#define SCTL4_15_CHSEL(value)           BIT_FIELD((value), 0)
#define SCTL4_15_CHSEL_MASK             BIT_FIELD(MASK(4), 0)
#define SCTL4_15_CHSEL_VALUE(reg) \
    FIELD_VALUE((reg), SCTL4_15_CHSEL_MASK, 0)

#define SCTL4_15_EXTREN                 BIT(4)
#define SCTL4_15_EXTFEN                 BIT(5)

#define SCTL4_15_TRGDLYDIV(value)       BIT_FIELD((value), 6)
#define SCTL4_15_TRGDLYDIV_MASK         BIT_FIELD(MASK(2), 6)
#define SCTL4_15_TRGDLYDIV_VALUE(reg) \
    FIELD_VALUE((reg), SCTL4_15_TRGDLYDIV_MASK, 6)

#define SCTL4_15_TRGDLYCNT(value)       BIT_FIELD((value), 8)
#define SCTL4_15_TRGDLYCNT_MASK         BIT_FIELD(MASK(8), 8)
#define SCTL4_15_TRGDLYCNT_VALUE(reg) \
    FIELD_VALUE((reg), SCTL4_15_TRGDLYCNT_MASK, 8)

#define SCTL4_15_TRGSEL(value)          BIT_FIELD((value), 16)
#define SCTL4_15_TRGSEL_MASK            BIT_FIELD(MASK(5), 16)
#define SCTL4_15_TRGSEL_VALUE(reg) \
    FIELD_VALUE((reg), SCTL4_15_TRGSEL_MASK, 16)

#define SCTL4_15_INTPOS                 BIT(22)

#define SCTL4_15_EXTSMPT(value)         BIT_FIELD((value), 24)
#define SCTL4_15_EXTSMPT_MASK           BIT_FIELD(MASK(8), 24)
#define SCTL4_15_EXTSMPT_VALUE(reg) \
    FIELD_VALUE((reg), SCTL4_15_EXTSMPT_MASK, 24)
/*------------------Sample Module 16..18 control registers--------------------*/
#define SCTL16_18_EXTSMPT(value)        BIT_FIELD((value), 24)
#define SCTL16_18_EXTSMPT_MASK          BIT_FIELD(MASK(8), 24)
#define SCTL16_18_EXTSMPT_VALUE(reg) \
    FIELD_VALUE((reg), SCTL16_18_EXTSMPT_MASK, 24)
/*------------------Result Compare registers----------------------------------*/
#define CMP_ADCMPEN                     BIT(0)
#define CMP_ADCMPIE                     BIT(1)
#define CMP_CMPCOND                     BIT(2)

#define CMP_CMPSPL(value)               BIT_FIELD((value), 3)
#define CMP_CMPSPL_MASK                 BIT_FIELD(MASK(5), 3)
#define CMP_CMPSPL_VALUE(reg)           FIELD_VALUE((reg), CMP_CMPSPL_MASK, 3)

#define CMP_CMPMCNT(value)              BIT_FIELD((value), 8)
#define CMP_CMPMCNT_MASK                BIT_FIELD(MASK(4), 8)
#define CMP_CMPMCNT_VALUE(reg)          FIELD_VALUE((reg), CMP_CMPMCNT_MASK, 8)

#define CMP_CMPWEN                      BIT(15)

#define CMP_CMPDAT(value)               BIT_FIELD((value), 16)
#define CMP_CMPDAT_MASK                 BIT_FIELD(MASK(12), 16)
#define CMP_CMPDAT_VALUE(reg)           FIELD_VALUE((reg), CMP_CMPDAT_MASK, 16)
/*------------------Status registers 0 and 1----------------------------------*/
#define STATUS0_1_VALID(channel)        BIT(channel)
#define STATUS0_1_VALID_MASK            BIT_FIELD(MASK(16), 0)
#define STATUS0_1_VALID_VALUE(reg) \
    FIELD_VALUE((reg), STATUS0_1_VALID_MASK, 0)

#define STATUS0_1_OV(channel)           BIT((channel) + 16)
#define STATUS0_1_OV_MASK               BIT_FIELD(MASK(16), 16)
#define STATUS0_1_OV_VALUE(reg) \
    FIELD_VALUE((reg), STATUS0_1_OV_MASK, 16)
/*------------------Status register 2-----------------------------------------*/
#define STATUS2_ADIF_MASK               BIT_FIELD(MASK(4), 0)
#define STATUS2_ADIF0                   BIT(0)
#define STATUS2_ADIF1                   BIT(1)
#define STATUS2_ADIF2                   BIT(2)
#define STATUS2_ADIF3                   BIT(3)
#define STATUS2_ADCMPF_MASK             BIT_FIELD(MASK(4), 4)
#define STATUS2_ADCMPF0                 BIT(4)
#define STATUS2_ADCMPF1                 BIT(5)
#define STATUS2_ADCMPF2                 BIT(6)
#define STATUS2_ADCMPF3                 BIT(7)
#define STATUS2_ADOVIF_MASK             BIT_FIELD(MASK(4), 8)
#define STATUS2_ADOVIF0                 BIT(8)
#define STATUS2_ADOVIF1                 BIT(9)
#define STATUS2_ADOVIF2                 BIT(10)
#define STATUS2_ADOVIF3                 BIT(11)
#define STATUS2_ADCMPO0                 BIT(12)
#define STATUS2_ADCMPO1                 BIT(13)
#define STATUS2_ADCMPO2                 BIT(14)
#define STATUS2_ADCMPO3                 BIT(15)

#define STATUS2_CHANNEL(value)          BIT_FIELD((value), 16)
#define STATUS2_CHANNEL_MASK            BIT_FIELD(MASK(5), 16)
#define STATUS2_CHANNEL_VALUE(reg) \
    FIELD_VALUE((reg), STATUS2_CHANNEL_MASK, 16)

#define STATUS2_BUSY                    BIT(23)
#define STATUS2_ADOVIF                  BIT(24)
#define STATUS2_STOVF                   BIT(25)
#define STATUS2_AVALID                  BIT(26)
#define STATUS2_AOV                     BIT(27)
/*------------------Power Management register---------------------------------*/
enum
{
  PWDMOD_DEEP_POWER_DOWN  = 0,
  PWDMOD_POWER_DOWN       = 1,
  PWDMOD_STANDBY          = 2
};

#define PWRM_PWUPRDY                    BIT(0)
#define PWRM_PWUCALEN                   BIT(1)

#define PWRM_PWDMOD(value)              BIT_FIELD((value), 2)
#define PWRM_PWDMOD_MASK                BIT_FIELD(MASK(2), 2)
#define PWRM_PWDMOD_VALUE(reg)          FIELD_VALUE((reg), PWRM_PWDMOD_MASK, 2)

#define PWRM_LDOSUT(value)              BIT_FIELD((value), 8)
#define PWRM_LDOSUT_MASK                BIT_FIELD(MASK(12), 8)
#define PWRM_LDOSUT_VALUE(reg)          FIELD_VALUE((reg), PWRM_LDOSUT_MASK, 8)
/*------------------Calibration Control register------------------------------*/
#define CALCTL_CALSTART                 BIT(1)
#define CALCTL_CALDONE                  BIT(2)
#define CALCTL_CALSEL                   BIT(3)
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_NUMICRO_EADC_DEFS_H_ */
