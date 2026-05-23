/*
 * halm/core/riscv/irq.h
 * Copyright (C) 2026 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_IRQ_H_
#error This header should not be included directly
#endif

#ifndef HALM_CORE_RISCV_IRQ_H_
#define HALM_CORE_RISCV_IRQ_H_
/*----------------------------------------------------------------------------*/
#include <stdint.h>
/*----------------------------------------------------------------------------*/
typedef uint16_t IrqNumber;
typedef uint8_t IrqPriority;
typedef uint32_t IrqState;
/*----------------------------------------------------------------------------*/
#undef HEADER_PATH
#define HEADER_PATH <halm/platform/PLATFORM_TYPE/irq.h>
#include HEADER_PATH

#undef HEADER_PATH
#define HEADER_PATH <halm/platform/PLATFORM_TYPE/vectors.h>
#include HEADER_PATH
/*----------------------------------------------------------------------------*/
#endif /* HALM_CORE_RISCV_IRQ_H_ */
