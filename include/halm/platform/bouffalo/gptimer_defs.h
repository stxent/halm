/*
 * halm/platform/bouffalo/gptimer_defs.h
 * Copyright (C) 2026 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_BOUFFALO_GPTIMER_DEFS_H_
#define HALM_PLATFORM_BOUFFALO_GPTIMER_DEFS_H_
/*----------------------------------------------------------------------------*/
#include <xcore/bits.h>
/*----------------------------------------------------------------------------*/
#undef HEADER_PATH
#define HEADER_PATH <halm/platform/PLATFORM_TYPE/PLATFORM/gptimer_defs.h>
#include HEADER_PATH
#undef HEADER_PATH
/*------------------Timer Match Register Status-------------------------------*/
#define TMSR_STATUS(match)              BIT(match)
/*------------------Timer Match Interrupt Enable Register---------------------*/
#define TIER_ENABLE(match)              BIT(match)
#define TIER_MASK                       BIT_FIELD(MASK(3), 0)
/*------------------Timer Pre-Load Control Register---------------------------*/
enum
{
  PLCR_NO_PRELOAD = 0,
  PLCR_MATCH0     = 1,
  PLCR_MATCH1     = 2,
  PLCR_MATCH2     = 3
};

#define TPLCR_TPLCR(value)              BIT_FIELD((value), 0)
#define TPLCR_TPLCR_MASK                BIT_FIELD(MASK(2), 0)
#define TPLCR_TPLCR_VALUE(reg)          FIELD_VALUE((reg), TPLCR_TPLCR_MASK, 0)
/*------------------Timer Interrupt Clear Control Register--------------------*/
#define TICR_CLEAR(match)               BIT(match)
#define TICR_MASK                       BIT_FIELD(MASK(3), 0)
/*------------------Timer Count Enable Register-------------------------------*/
#define TCER_ENABLE(channel)            BIT((channel) + 1)
/*------------------Timer Count Mode Register---------------------------------*/
#define TCMR_MASK(channel)              BIT((channel) + 1)
#define TCMR_PRELOAD(channel)           0
#define TCMR_FREERUN(channel)           BIT((channel) + 1)
/*------------------Timer Match Interrupt Mode Register-----------------------*/
#define TILR_MASK(channel)              BIT((channel) + 1)
#define TILR_LEVEL(channel)             0
#define TILR_PULSE(channel)             BIT((channel) + 1)
/*------------------WDT/Timer Clock Division Register-------------------------*/
#define TCDR_TCDR2(value)               BIT_FIELD((value), 8)
#define TCDR_TCDR2_MASK                 BIT_FIELD(MASK(8), 8)
#define TCDR_TCDR2_VALUE(reg)           FIELD_VALUE((reg), TCDR_TCDR2_MASK, 8)

#define TCDR_TCDR3(value)               BIT_FIELD((value), 16)
#define TCDR_TCDR3_MASK                 BIT_FIELD(MASK(8), 16)
#define TCDR_TCDR3_VALUE(reg)           FIELD_VALUE((reg), TCDR_TCDR3_MASK, 16)

#define TCDR_WCDR(value)                BIT_FIELD((value), 24)
#define TCDR_WCDR_MASK                  BIT_FIELD(MASK(8), 24)
#define TCDR_WCDR_VALUE(reg)            FIELD_VALUE((reg), TCDR_WCDR_MASK, 24)

#define TCDR_CDR(channel, value)        BIT_FIELD((value), (channel) * 8 + 8)
#define TCDR_CDR_MASK(channel)          BIT_FIELD(MASK(8), (channel) * 8 + 8)
#define TCDR_CDR_VALUE(channel, reg) \
    FIELD_VALUE((reg), TCDR_CDR_MASK(channel), (channel) * 8 + 8)
#define TCDR_CDR_MAX                    255
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_BOUFFALO_GPTIMER_DEFS_H_ */
