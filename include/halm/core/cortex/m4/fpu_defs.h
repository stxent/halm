/*
 * halm/core/cortex/fpu_defs.h
 * Copyright (C) 2015 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef HALM_CORE_CORTEX_FPU_DEFS_H_
#define HALM_CORE_CORTEX_FPU_DEFS_H_
/*----------------------------------------------------------------------------*/
#include <xcore/bits.h>
/*------------------Coprocessor Access Control Register-----------------------*/
enum
{
  CPACR_ACCESS_DENIED     = 0,
  CPACR_PRIVILEGED_ACCESS = 1,
  CPACR_FULL_ACCESS       = 3,
};

#define CPACR_CP10_MASK                 BIT_FIELD(MASK(2), 20)
#define CPACR_CP10(value)               BIT_FIELD((value), 20)
#define CPACR_CP10_VALUE(reg)           FIELD_VALUE((reg), CPACR_CP10_MASK, 20)
#define CPACR_CP11_MASK                 BIT_FIELD(MASK(2), 22)
#define CPACR_CP11(value)               BIT_FIELD((value), 22)
#define CPACR_CP11_VALUE(reg)           FIELD_VALUE((reg), CPACR_CP11_MASK, 22)
/*------------------Floating Point Context Control Register-------------------*/
#define FPCCR_LSPACT                    BIT(0)
#define FPCCR_USER                      BIT(1)
#define FPCCR_THREAD                    BIT(3)
#define FPCCR_HFRDY                     BIT(4)
#define FPCCR_MMRDY                     BIT(5)
#define FPCCR_BFRDY                     BIT(6)
#define FPCCR_MONRDY                    BIT(8)
#define FPCCR_LSPEN                     BIT(30)
#define FPCCR_ASPEN                     BIT(31)
/*------------------Floating Point Status Control Register--------------------*/
enum
{
  FPSCR_RMODE_RN = 0, /* Round to nearest */
  FPSCR_RMODE_RP = 1, /* Round towards Plus Infinity */
  FPSCR_RMODE_RM = 2, /* Round towards Minus Infinity */
  FPSCR_RMODE_RZ = 3 /* Round towards Zero */
};

#define FPSCR_IOC                       BIT(0)
#define FPSCR_DZC                       BIT(1)
#define FPSCR_OFC                       BIT(2)
#define FPSCR_UFC                       BIT(3)
#define FPSCR_IXC                       BIT(4)
#define FPSCR_IDC                       BIT(7)
#define FPSCR_RMODE_MASK                BIT_FIELD(MASK(2), 22)
#define FPSCR_RMODE(value)              BIT_FIELD((value), 22)
#define FPSCR_RMODE_VALUE(reg)          FIELD_VALUE((reg), FPSCR_RMODE_MASK, 22)
#define FPSCR_FZ                        BIT(24)
#define FPSCR_DN                        BIT(25)
#define FPSCR_AHP                       BIT(26)
/*----------------------------------------------------------------------------*/
#endif /* HALM_CORE_CORTEX_FPU_DEFS_H_ */
