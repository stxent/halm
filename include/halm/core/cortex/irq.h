/*
 * halm/core/cortex/irq.h
 * Copyright (C) 2013 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef HALM_CORE_CORTEX_IRQ_H_
#define HALM_CORE_CORTEX_IRQ_H_
/*----------------------------------------------------------------------------*/
#include <stdint.h>
/*----------------------------------------------------------------------------*/
typedef int16_t IrqNumber;
typedef uint8_t IrqPriority;
typedef uint32_t IrqState;
/*----------------------------------------------------------------------------*/
#undef HEADER_PATH
#define HEADER_PATH <halm/core/CORE_TYPE/CORE/irq.h>
#include HEADER_PATH

#undef HEADER_PATH
#define HEADER_PATH <halm/platform/PLATFORM_TYPE/vectors.h>
#include HEADER_PATH
/*----------------------------------------------------------------------------*/
#endif /* HALM_CORE_CORTEX_IRQ_H_ */
