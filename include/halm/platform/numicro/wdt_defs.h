/*
 * halm/platform/numicro/wdt_defs.h
 * Copyright (C) 2023 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_NUMICRO_WDT_DEFS_H_
#define HALM_PLATFORM_NUMICRO_WDT_DEFS_H_
/*----------------------------------------------------------------------------*/
#include <xcore/bits.h>
/*------------------Control register------------------------------------------*/
enum
{
  TOUTSEL_16      = 0,
  TOUTSEL_64      = 1,
  TOUTSEL_256     = 2,
  TOUTSEL_1024    = 3,
  TOUTSEL_4096    = 4,
  TOUTSEL_16384   = 5,
  TOUTSEL_65536   = 6,
  TOUTSEL_262144  = 7
};

#define CTL_RSTCNT                      BIT(0)
#define CTL_RSTEN                       BIT(1)
#define CTL_RSTF                        BIT(2)
#define CTL_IF                          BIT(3)
#define CTL_WKEN                        BIT(4)
#define CTL_WKF                         BIT(5)
#define CTL_INTEN                       BIT(6)
#define CTL_WDTEN                       BIT(7)

#define CTL_TOUTSEL(value)              BIT_FIELD((value), 8)
#define CTL_TOUTSEL_MASK                BIT_FIELD(MASK(3), 8)
#define CTL_TOUTSEL_VALUE(reg)          FIELD_VALUE((reg), CTL_TOUTSEL_MASK, 8)

#define CTL_SYNC                        BIT(30)
#define CTL_ICEDEBUG                    BIT(31)
/*------------------Alternative Control register------------------------------*/
enum
{
  RSTDSEL_3     = 0,
  RSTDSEL_18    = 1,
  RSTDSEL_130   = 2,
  RSTDSEL_1026  = 3
};

#define ALTCTL_RSTDSEL(value)           BIT_FIELD((value), 0)
#define ALTCTL_RSTDSEL_MASK             BIT_FIELD(MASK(2), 0)
#define ALTCTL_RSTDSEL_VALUE(reg) \
    FIELD_VALUE((reg), ALTCTL_RSTDSEL_MASK, 0)
/*------------------Reset Counter register------------------------------------*/
#define RSTCNT_MAGIC_NUMBER             0x00005AA5
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_NUMICRO_WDT_DEFS_H_ */
