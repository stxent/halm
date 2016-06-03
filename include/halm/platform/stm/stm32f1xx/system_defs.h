/*
 * halm/platform/stm/stm32f1xx/system_defs.h
 * Copyright (C) 2016 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef HALM_PLATFORM_STM_STM32F1XX_SYSTEM_DEFS_H_
#define HALM_PLATFORM_STM_STM32F1XX_SYSTEM_DEFS_H_
/*----------------------------------------------------------------------------*/
#include <xcore/bits.h>
/*----------------------------------------------------------------------------*/
#define ACR_LATENCY(value)              BIT_FIELD((value), 0)
#define ACR_LATENCY_MASK                BIT_FIELD(MASK(3), 0)
#define ACR_LATENCY_VALUE(reg)          FIELD_VALUE((reg), ACR_LATENCY_MASK, 0)
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_STM_STM32F1XX_SYSTEM_DEFS_H_ */
