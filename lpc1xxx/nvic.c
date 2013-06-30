/*
 * nvic.c
 * Copyright (C) 2013 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include "nvic.h"
/*----------------------------------------------------------------------------*/
void nvicSetPriority(enum interrupt irq, uint8_t priority)
{
  if (irq < 0)
    SCB->SHP[(irq & 0x0F) - 4] = priority;
  else
    NVIC->IP[irq] = priority;
}
/*----------------------------------------------------------------------------*/
uint8_t nvicGetPriority(enum interrupt irq)
{
  if (irq < 0)
    return SCB->SHP[(irq & 0x0F) - 4];
  else
    return NVIC->IP[irq];
}
/*----------------------------------------------------------------------------*/
void nvicSetPriorityGrouping(uint8_t subpriority)
{
  /* TODO */
}
