/*
 * halm/core/cortex/m3/nvic.h
 * Copyright (C) 2017 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef HALM_CORE_CORTEX_M3_NVIC_H_
#define HALM_CORE_CORTEX_M3_NVIC_H_
/*----------------------------------------------------------------------------*/
#include <xcore/helpers.h>
#include <halm/core/core_defs.h>
/*----------------------------------------------------------------------------*/
BEGIN_DECLS

uint8_t nvicGetPriorityGrouping(void);
void nvicSetPriorityGrouping(uint8_t);
void nvicResetCore(void);
void nvicSetVectorTableOffset(uint32_t);

END_DECLS
/*----------------------------------------------------------------------------*/
#endif /* HALM_CORE_CORTEX_M3_NVIC_H_ */
