/*
 * halm/platform/numicro/gen_2/gptimer_defs.h
 * Copyright (C) 2023 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_NUMICRO_GPTIMER_DEFS_H_
#error This header should not be included directly
#endif

#ifndef HALM_PLATFORM_NUMICRO_GEN_2_GPTIMER_DEFS_H_
#define HALM_PLATFORM_NUMICRO_GEN_2_GPTIMER_DEFS_H_
/*----------------------------------------------------------------------------*/
#include <xcore/bits.h>
/*------------------Control register------------------------------------------*/
enum
{
  OPMODE_ONE_SHOT       = 0,
  OPMODE_PERIODIC       = 1,
  OPMODE_TOGGLE_OUTPUT  = 2,
  OPMODE_CONTINUOUS     = 3
};

#define CTL_PSC(value)                  BIT_FIELD((value), 0)
#define CTL_PSC_MASK                    BIT_FIELD(MASK(8), 0)
#define CTL_PSC_VALUE(reg)              FIELD_VALUE((reg), CTL_PSC_MASK, 0)
#define CTL_PSC_WIDTH                   8

#define CTL_INTRGEN                     BIT(19)
#define CTL_PERIOSEL                    BIT(20)
#define CTL_TGLPINSEL                   BIT(21)
#define CTL_CAPSRC                      BIT(22)
#define CTL_WKEN                        BIT(23)
#define CTL_EXTCNTEN                    BIT(24)
#define CTL_ACTSTS                      BIT(25)

#define CTL_OPMODE(value)               BIT_FIELD((value), 27)
#define CTL_OPMODE_MASK                 BIT_FIELD(MASK(2), 27)
#define CTL_OPMODE_VALUE(reg)           FIELD_VALUE((reg), CTL_OPMODE_MASK, 27)

#define CTL_INTEN                       BIT(29)
#define CTL_CNTEN                       BIT(30)
#define CTL_ICEDEBUG                    BIT(31)
/*------------------Comparator register---------------------------------------*/
#define CMP_CMPDAT_MASK                 BIT_FIELD(MASK(24), 0)
/*------------------Interrupt Status register---------------------------------*/
#define INTSTS_TIF                      BIT(0)
#define INTSTS_TWKF                     BIT(1)
/*------------------Data register---------------------------------------------*/
#define CNT_RSTACT                      BIT(31)

#define CNT_CNT_MASK                    BIT_FIELD(MASK(24), 0)
#define CNT_CNT_VALUE(reg)              FIELD_VALUE((reg), CNT_CNT_MASK, 0)
/*------------------External Control register---------------------------------*/
enum
{
  CAPEDGE_FALLING         = 0,
  CAPEDGE_RISING          = 1,
  CAPEDGE_FALLING_FIRST   = 2,
  CAPEDGE_RISING_FIRST    = 3,
  CAPEDGE_FALLING_RISING  = 6,
  CAPEDGE_RISING_FALLING  = 7
};

enum
{
  ICAPSEL_ACMP0 = 0,
  ICAPSEL_ACMP1 = 1,
  ICAPSEL_HXT   = 2,
  ICAPSEL_LXT   = 3,
  ICAPSEL_HIRC  = 4,
  ICAPSEL_LIRC  = 5
};

#define EXTCTL_CNTPHASE                 BIT(0)
#define EXTCTL_CAPEN                    BIT(3)
#define EXTCTL_CAPFUNCS                 BIT(4)
#define EXTCTL_CAPIEN                   BIT(5)
#define EXTCTL_CAPDBEN                  BIT(6)
#define EXTCTL_CNTDBEN                  BIT(7)

#define EXTCTL_ICAPSEL(value)           BIT_FIELD((value), 8)
#define EXTCTL_ICAPSEL_MASK             BIT_FIELD(MASK(3), 8)
#define EXTCTL_ICAPSEL_VALUE(reg) \
    FIELD_VALUE((reg), EXTCTL_ICAPSEL_MASK, 8)

#define EXTCTL_CAPEDGE(value)           BIT_FIELD((value), 12)
#define EXTCTL_CAPEDGE_MASK             BIT_FIELD(MASK(3), 12)
#define EXTCTL_CAPEDGE_VALUE(reg) \
    FIELD_VALUE((reg), EXTCTL_CAPEDGE_MASK, 12)

#define EXTCTL_ECNTSSEL                 BIT(16)

#define EXTCTL_CAPDIVSCL(value)         BIT_FIELD((value), 28)
#define EXTCTL_CAPDIVSCL_MASK           BIT_FIELD(MASK(4), 28)
#define EXTCTL_CAPDIVSCL_VALUE(reg) \
    FIELD_VALUE((reg), EXTCTL_CAPDIVSCL_MASK, 28)
/*------------------External Interrupt Status register------------------------*/
#define EINTSTS_CAPIF                   BIT(0)
/*------------------Trigger Control register----------------------------------*/
#define TRGCTL_TRGSSEL                  BIT(0)
#define TRGCTL_TRGPWM                   BIT(1)
#define TRGCTL_TRGEADC                  BIT(2)
#define TRGCTL_TRGDAC                   BIT(3)
#define TRGCTL_TRGPDMA                  BIT(4)
/*------------------Alternative Control register------------------------------*/
#define ALTCTL_FUNCSEL                  BIT(0)
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_NUMICRO_GEN_2_GPTIMER_DEFS_H_ */
