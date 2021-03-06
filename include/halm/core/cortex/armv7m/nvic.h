/*
 * halm/core/cortex/armv7m/nvic.h
 * Copyright (C) 2017 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_CORE_CORTEX_NVIC_H_
#error This header should not be included directly
#endif

#ifndef HALM_CORE_CORTEX_ARMV7M_NVIC_H_
#define HALM_CORE_CORTEX_ARMV7M_NVIC_H_
/*----------------------------------------------------------------------------*/
#include <xcore/helpers.h>
#include <stdint.h>
/*----------------------------------------------------------------------------*/
BEGIN_DECLS

uint8_t nvicGetPriorityGrouping(void);
void nvicSetPriorityGrouping(uint8_t);
void nvicResetCore(void);
void nvicSetVectorTableOffset(uint32_t);

END_DECLS
/*----------------------------------------------------------------------------*/
#endif /* HALM_CORE_CORTEX_ARMV7M_NVIC_H_ */
