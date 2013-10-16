/*
 * irq.h
 * Copyright (C) 2013 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef IRQ_TOP_H_
#define IRQ_TOP_H_
/*----------------------------------------------------------------------------*/
#include <stdint.h>
#include <mcu.h>
/*----------------------------------------------------------------------------*/
typedef int16_t irq_t;
/*----------------------------------------------------------------------------*/
#define HEADER_PATH <core/CORE_TYPE/irq.h>
#include HEADER_PATH
#undef HEADER_PATH
/*----------------------------------------------------------------------------*/
#define HEADER_PATH <platform/PLATFORM_TYPE/vectors.h>
#include HEADER_PATH
#undef HEADER_PATH
/*----------------------------------------------------------------------------*/
#endif /* IRQ_TOP_H_ */
