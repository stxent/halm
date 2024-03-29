/*
 * halm/platform/numicro/m03x/system_defs.h
 * Copyright (C) 2023 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_NUMICRO_M03X_SYSTEM_DEFS_H_
#define HALM_PLATFORM_NUMICRO_M03X_SYSTEM_DEFS_H_
/*----------------------------------------------------------------------------*/
#include <xcore/bits.h>
/*------------------Multiple Function Control registers-----------------------*/
#define MFP_FUNCTION_MASK(pin)          BIT_FIELD(MASK(4), (pin) * 4)
#define MFP_FUNCTION(value, pin)        BIT_FIELD((value), (pin) * 4)
/*------------------Register Lock Control register----------------------------*/
#define REGLCTL_MAGIC_NUMBER_0          0x59
#define REGLCTL_MAGIC_NUMBER_1          0x16
#define REGLCTL_MAGIC_NUMBER_2          0x88
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_NUMICRO_M03X_SYSTEM_DEFS_H_ */
