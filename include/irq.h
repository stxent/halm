/*
 * irq.h
 * Copyright (C) 2013 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef HALM_IRQ_H_
#define HALM_IRQ_H_
/*----------------------------------------------------------------------------*/
#include <stdint.h>
#include <libhalm/target.h>
/*----------------------------------------------------------------------------*/
typedef int16_t irqNumber;
typedef uint8_t irqPriority;
typedef uint32_t irqState;
/*----------------------------------------------------------------------------*/
#undef HEADER_PATH
#define HEADER_PATH <core/CORE_TYPE/irq.h>
#include HEADER_PATH
#undef HEADER_PATH
#define HEADER_PATH <platform/PLATFORM_TYPE/vectors.h>
#include HEADER_PATH
#undef HEADER_PATH
/*----------------------------------------------------------------------------*/
#endif /* HALM_IRQ_H_ */
