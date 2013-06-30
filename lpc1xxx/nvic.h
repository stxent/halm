/*
 * nvic.h
 * Copyright (C) 2013 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef NVIC_H_
#define NVIC_H_
/*----------------------------------------------------------------------------*/
#include <stdint.h>
#include <LPC13xx.h>
#include "interrupts.h"
/*----------------------------------------------------------------------------*/
uint8_t nvicGetPriority(enum interrupt);
void nvicSetPriority(enum interrupt, uint8_t);
void nvicSetPriorityGrouping(uint8_t);
/*----------------------------------------------------------------------------*/
static inline void nvicEnable(enum interrupt irq)
{
  NVIC->ISER[irq >> 5] = 1 << (irq & 0x1F);
}
/*----------------------------------------------------------------------------*/
static inline void nvicDisable(enum interrupt irq)
{
  NVIC->ICER[irq >> 5] = 1 << (irq & 0x1F);
}
/*----------------------------------------------------------------------------*/
#endif /* NVIC_H_ */
