/*
 * halm/core/cortex/irq.h
 * Copyright (C) 2013 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_IRQ_H_
#error This header should not be included directly
#endif

#ifndef HALM_CORE_CORTEX_IRQ_H_
#define HALM_CORE_CORTEX_IRQ_H_
/*----------------------------------------------------------------------------*/
#include <stdint.h>
/*----------------------------------------------------------------------------*/
#define IRQ_PRIORITY_TO_REG(priority) \
    ((((1 << NVIC_IRQ_BITS) - 1) - (priority)) << (8 - NVIC_IRQ_BITS))
#define IRQ_REG_TO_PRIORITY(reg) \
    (((1 << NVIC_IRQ_BITS) - 1) - ((reg) >> (8 - NVIC_IRQ_BITS)))

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
