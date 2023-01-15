/*
 * halm/platform/numicro/gptimer_defs.h
 * Copyright (C) 2023 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_NUMICRO_GPTIMER_DEFS_H_
#define HALM_PLATFORM_NUMICRO_GPTIMER_DEFS_H_
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

#define CTL_TRGPDMA                     BIT(8)
#define CTL_TRGBPWM                     BIT(9)
#define CTL_INTRGEN                     BIT(10)
#define CTL_CAPSRC                      BIT(16)
#define CTL_TRGSSEL                     BIT(18)
#define CTL_TRGPWM                      BIT(19)
#define CTL_TRGADC                      BIT(21)
#define CTL_TGLPINSEL                   BIT(22)
#define CTL_WKEN                        BIT(23)
#define CTL_EXTCNTEN                    BIT(24)
#define CTL_ACTSTS                      BIT(25)
#define CTL_RSTCNT                      BIT(26)

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
/*------------------External Control register---------------------------------*/
enum
{
  CAPEDGE_FALLING = 0,
  CAPEDGE_RISING  = 1,
  CAPEDGE_BOTH    = 2
};

enum
{
  INTERCAPSEL_ACMP0 = 0,
  INTERCAPSEL_ACMP1 = 1,
  INTERCAPSEL_LIRC  = 5
};

#define EXTCTL_CNTPHASE                 BIT(8)

#define EXTCTL_CAPEDGE(value)           BIT_FIELD((value), 1)
#define EXTCTL_CAPEDGE_MASK             BIT_FIELD(MASK(2), 1)
#define EXTCTL_CAPEDGE_VALUE(reg) \
    FIELD_VALUE((reg), EXTCTL_CAPEDGE_MASK, 1)

#define EXTCTL_CAPEN                    BIT(3)
#define EXTCTL_CAPFUNCS                 BIT(4)
#define EXTCTL_CAPIEN                   BIT(5)
#define EXTCTL_CAPDBEN                  BIT(6)
#define EXTCTL_CNTDBEN                  BIT(7)

#define EXTCTL_INTERCAPSEL(value)       BIT_FIELD((value), 8)
#define EXTCTL_INTERCAPSEL_MASK         BIT_FIELD(MASK(3), 8)
#define EXTCTL_INTERCAPSEL_VALUE(reg) \
    FIELD_VALUE((reg), EXTCTL_INTERCAPSEL_MASK, 8)

#define EXTCTL_ECNTSSEL                 BIT(16)
/*------------------External Interrupt Status register------------------------*/
#define EINTSTS_CAPIF                   BIT(0)
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_NUMICRO_GPTIMER_DEFS_H_ */
