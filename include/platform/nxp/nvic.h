/*
 * nvic.h
 * Copyright (C) 2013 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef NVIC_H_
#define NVIC_H_
/*----------------------------------------------------------------------------*/
#include <stdint.h>
#include "./device_defs.h"
/*----------------------------------------------------------------------------*/
typedef int16_t irq_t;
/*----------------------------------------------------------------------------*/
uint8_t nvicGetPriority(irq_t);
void nvicSetPriority(irq_t, uint8_t);
void nvicSetPriorityGrouping(uint8_t);
/*----------------------------------------------------------------------------*/
static inline void nvicEnable(irq_t irq)
{
  NVIC->ISER[irq >> 5] = 1 << (irq & 0x1F);
}
/*----------------------------------------------------------------------------*/
static inline void nvicDisable(irq_t irq)
{
  NVIC->ICER[irq >> 5] = 1 << (irq & 0x1F);
}
/*----------------------------------------------------------------------------*/
#endif /* NVIC_H_ */
