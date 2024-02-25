/*
 * halm/platform/imxrt/imxrt106x/system_defs.h
 * Copyright (C) 2024 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_IMXRT_IMXRT106X_SYSTEM_DEFS_H_
#define HALM_PLATFORM_IMXRT_IMXRT106X_SYSTEM_DEFS_H_
/*----------------------------------------------------------------------------*/
#include <xcore/bits.h>
/*------------------Clock Gating registers------------------------------------*/
enum
{
  CG_OFF          = 0,
  CG_ON_RUN_MODE  = 1,
  CG_ON_ALL_MODES = 3
};

#define CCGR_CG(value, channel)         BIT_FIELD((value), (channel) * 2)
#define CCGR_CG_MASK(channel)           BIT_FIELD(MASK(2), (channel) * 2)
#define CCGR_CG_VALUE(reg, channel) \
    FIELD_VALUE((reg), CCGR_CG_MASK(channel), (channel) * 2)
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_IMXRT_IMXRT106X_SYSTEM_DEFS_H_ */
