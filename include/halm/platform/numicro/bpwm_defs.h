/*
 * halm/platform/numicro/bpwm_defs.h
 * Copyright (C) 2023 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_NUMICRO_BPWM_DEFS_H_
#define HALM_PLATFORM_NUMICRO_BPWM_DEFS_H_
/*----------------------------------------------------------------------------*/
#include <xcore/bits.h>
/*------------------Control Register 0----------------------------------------*/
#define CTL0_CTRLD_MASK                 BIT_FIELD(MASK(6), 0)
#define CTL0_CTRLD(channel)             BIT((channel) + 0)
#define CTL0_IMMLDEN_MASK               BIT_FIELD(MASK(6), 16)
#define CTL0_IMMLDEN(channel)           BIT((channel) + 16)
#define CTL0_DBGHALT                    BIT(30)
#define CTL0_DBGTRIOFF                  BIT(31)
/*------------------Control Register 1----------------------------------------*/
enum
{
  CNTTYPE0_UP       = 0,
  CNTTYPE0_DOWN     = 1,
  CNTTYPE0_UP_DOWN  = 2
};

#define CTL1_CNTTYPE0(value)            BIT_FIELD((value), 0)
#define CTL1_CNTTYPE0_MASK              BIT_FIELD(MASK(2), 0)
#define CTL1_CNTTYPE0_VALUE(reg) \
    FIELD_VALUE((reg), CTL1_CNTTYPE0_MASK, 0)
/*------------------Clock Source register-------------------------------------*/
enum
{
  ECLKSRC0_BPWM_CLK   = 0,
  ECLKSRC0_TIMER0_OF  = 1,
  ECLKSRC0_TIMER1_OF  = 2,
  ECLKSRC0_TIMER2_OF  = 3,
  ECLKSRC0_TIMER3_OF  = 4
};

#define CLKSRC_ECLKSRC0(value)          BIT_FIELD((value), 0)
#define CLKSRC_ECLKSRC0_MASK            BIT_FIELD(MASK(3), 0)
#define CLKSRC_ECLKSRC0_VALUE(reg) \
    FIELD_VALUE((reg), CLKSRC_ECLKSRC0_MASK, 0)
/*------------------Clock Prescale register-----------------------------------*/
#define CLKPSC_MAX                      MASK(12)
/*------------------Counter Enable register-----------------------------------*/
#define CNTEN_CNTEN0                    BIT(0)
/*------------------Clear Counter register------------------------------------*/
#define CNTCLR_CNTCLR0                  BIT(0)
/*------------------Counter register------------------------------------------*/
#define CNT_CNT(value)                  BIT_FIELD((value), 0)
#define CNT_CNT_MASK                    BIT_FIELD(MASK(16), 0)
#define CNT_CNT_VALUE(reg)              FIELD_VALUE((reg), CNT_CNT_MASK, 0)

#define CNT_DIRF                        BIT(16)
/*------------------Generation Register 0-------------------------------------*/
enum
{
  PRDPCTL_NONE    = 0,
  PRDPCTL_LOW     = 1,
  PRDPCTL_HIGH    = 2,
  PRDPCTL_TOGGLE  = 3
};

enum
{
  ZPCTL_NONE    = 0,
  ZPCTL_LOW     = 1,
  ZPCTL_HIGH    = 2,
  ZPCTL_TOGGLE  = 3
};

#define WGCTL0_ZPCTL_MASK_ALL           BIT_FIELD(MASK(12), 0)
#define WGCTL0_ZPCTL(channel, value)    BIT_FIELD((value), (channel) * 2)
#define WGCTL0_ZPCTL_MASK(channel)      BIT_FIELD(MASK(2), (channel) * 2)
#define WGCTL0_ZPCTL_VALUE(channel, reg) \
    FIELD_VALUE((reg), WGCTL0_ZPCTL_MASK(channel), (channel) * 2)

#define WGCTL0_PRDPCTL_MASK_ALL         BIT_FIELD(MASK(12), 16)
#define WGCTL0_PRDPCTL(channel, value)  BIT_FIELD((value), (channel) * 2 + 16)
#define WGCTL0_PRDPCTL_MASK(channel)    BIT_FIELD(MASK(2), (channel) * 2 + 16)
#define WGCTL0_PRDPCTL_VALUE(channel, reg) \
    FIELD_VALUE((reg), WGCTL0_PRDPCTL_MASK(channel), (channel) * 2 + 16)
/*------------------Generation Register 1-------------------------------------*/
enum
{
  CMPDCTL_NONE    = 0,
  CMPDCTL_LOW     = 1,
  CMPDCTL_HIGH    = 2,
  CMPDCTL_TOGGLE  = 3
};

enum
{
  CMPUCTL_NONE    = 0,
  CMPUCTL_LOW     = 1,
  CMPUCTL_HIGH    = 2,
  CMPUCTL_TOGGLE  = 3
};

#define WGCTL1_CMPUCTL_MASK_ALL         BIT_FIELD(MASK(12), 0)
#define WGCTL1_CMPUCTL(channel, value)  BIT_FIELD((value), (channel) * 2)
#define WGCTL1_CMPUCTL_MASK(channel)    BIT_FIELD(MASK(2), (channel) * 2)
#define WGCTL1_CMPUCTL_VALUE(channel, reg) \
    FIELD_VALUE((reg), WGCTL1_CMPUCTL_MASK(channel), (channel) * 2)

#define WGCTL1_CMPDCTL_MASK_ALL         BIT_FIELD(MASK(12), 16)
#define WGCTL1_CMPDCTL(channel, value)  BIT_FIELD((value), (channel) * 2 + 16)
#define WGCTL1_CMPDCTL_MASK(channel)    BIT_FIELD(MASK(2), (channel) * 2 + 16)
#define WGCTL1_CMPDCTL_VALUE(channel, reg) \
    FIELD_VALUE((reg), WGCTL1_CMPDCTL_MASK(channel), (channel) * 2 + 16)
/*------------------Mask Data register----------------------------------------*/
#define MSK_MSKDAT_MASK                 BIT_FIELD(MASK(6), 0)
#define MSK_MSKDAT(channel)             BIT((channel) + 0)
/*------------------Pin Polarity Inverse register-----------------------------*/
#define POLCTL_PINV_MASK                BIT_FIELD(MASK(6), 0)
#define POLCTL_PINV(channel)            BIT((channel) + 0)
/*------------------Output Enable register------------------------------------*/
#define POEN_POEN_MASK                  BIT_FIELD(MASK(6), 0)
#define POEN_POEN(channel)              BIT((channel) + 0)
/*------------------Interrupt Enable register---------------------------------*/
#define INTEN_ZIEN0                     BIT(0)
#define INTEN_PIEN0                     BIT(8)
#define INTEN_CMPUIEN_MASK              BIT_FIELD(MASK(6), 16)
#define INTEN_CMPUIEN(channel)          BIT((channel) + 16)
#define INTEN_CMPDIEN_MASK              BIT_FIELD(MASK(6), 24)
#define INTEN_CMPDIEN(channel)          BIT((channel) + 24)
/*------------------Interrupt Enable register---------------------------------*/
#define INTSTS_ZIF0                     BIT(0)
#define INTSTS_PIF0                     BIT(8)
#define INTSTS_CMPUIF_MASK              BIT_FIELD(MASK(6), 16)
#define INTSTS_CMPUIF(channel)          BIT((channel) + 16)
#define INTSTS_CMPDIF_MASK              BIT_FIELD(MASK(6), 24)
#define INTSTS_CMPDIF(channel)          BIT((channel) + 24)
/*------------------Tigger EADC0/1 Source Select Registers 0/1----------------*/
enum
{
  /* CHx points */
  TRGSEL_ZERO             = 0,
  TRGSEL_PERIOD           = 1,
  TRGSEL_ZERO_PERIOD      = 2,
  TRGSEL_UP_COUNT         = 3,
  TRGSEL_DOWN_COUNT       = 4,
  /* Complementary CHx points */
  TRGSEL_UP_COUNT_COMP    = 8,
  TRGSEL_DOWN_COUNT_COMP  = 9
};

#define EADCTS_TRGEN(channel)           BIT((channel & 3) * 8 + 7)

#define EADCTS_TRGSEL(channel, value)   BIT_FIELD((value), (channel & 3) * 8)
#define EADCTS_TRGSEL_MASK(channel)     BIT_FIELD(MASK(4), (channel & 3) * 8)
#define EADCTS_TRGSEL_VALUE(channel, reg) \
    FIELD_VALUE((reg), EADCTS_TRGSEL_MASK(channel), (channel & 3) * 8)
/*------------------Synchronous Start Control register------------------------*/
enum
{
  SSRC_PWM0   = 0,
  SSRC_PWM1   = 1,
  SSRC_BPWM0  = 2,
  SSRC_BPWM1  = 3
};

#define SSCTL_SSEN0                     BIT(0)

#define SSCTL_SSRC(value)               BIT_FIELD((value), 8)
#define SSCTL_SSRC_MASK                 BIT_FIELD(MASK(2), 8)
#define SSCTL_SSRC_VALUE(reg)           FIELD_VALUE((reg), SSCTL_SSRC_MASK, 8)
/*------------------Synchronous Start Trigger register------------------------*/
#define SSTRG_CNTSEN                    BIT(0)
/*------------------Status register-------------------------------------------*/
#define STATUS_CNTMAXF0                 BIT(0)
#define STATUS_EADCTRG_MASK             BIT_FIELD(MASK(6), 16)
#define STATUS_EADCTRG(channel)         BIT((channel) + 16)
/*------------------Capture Input Enable register-----------------------------*/
#define CAPINEN_CAPINEN_MASK            BIT_FIELD(MASK(6), 0)
#define CAPINEN_CAPINEN(channel)        BIT((channel) + 0)
/*------------------Capture Control register----------------------------------*/
#define CAPCTL_CAPEN_MASK               BIT_FIELD(MASK(6), 0)
#define CAPCTL_CAPEN(channel)           BIT((channel) + 0)
#define CAPCTL_CAPINV_MASK              BIT_FIELD(MASK(6), 8)
#define CAPCTL_CAPINV(channel)          BIT((channel) + 8)
#define CAPCTL_RCRLDEN_MASK             BIT_FIELD(MASK(6), 16)
#define CAPCTL_RCRLDEN(channel)         BIT((channel) + 16)
#define CAPCTL_FCRLDEN_MASK             BIT_FIELD(MASK(6), 24)
#define CAPCTL_FCRLDEN(channel)         BIT((channel) + 24)
/*------------------Capture Status register-----------------------------------*/
#define CAPSTS_CRIFOV_MASK              BIT_FIELD(MASK(6), 0)
#define CAPSTS_CRIFOV(channel)          BIT((channel) + 0)
#define CAPSTS_CFIFOV_MASK              BIT_FIELD(MASK(6), 8)
#define CAPSTS_CFIFOV(channel)          BIT((channel) + 8)
/*------------------Capture Interrupt Enable register-------------------------*/
#define CAPIEN_CAPRIEN_MASK             BIT_FIELD(MASK(6), 0)
#define CAPIEN_CAPRIEN(channel)         BIT((channel) + 0)
#define CAPIEN_CAPFIEN_MASK             BIT_FIELD(MASK(6), 8)
#define CAPIEN_CAPFIEN(channel)         BIT((channel) + 8)
/*------------------Capture Interrupt Flag register---------------------------*/
#define CAPIF_CAPRIF_MASK               BIT_FIELD(MASK(6), 0)
#define CAPIF_CAPRIF(channel)           BIT((channel) + 0)
#define CAPIF_CAPFIF_MASK               BIT_FIELD(MASK(6), 8)
#define CAPIF_CAPFIF(channel)           BIT((channel) + 8)
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_NUMICRO_BPWM_DEFS_H_ */
