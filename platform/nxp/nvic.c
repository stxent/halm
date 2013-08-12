/*
 * nvic.c
 * Copyright (C) 2013 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include "platform/nxp/nvic.h"
/*----------------------------------------------------------------------------*/
void nvicSetPriority(irq_t irq, uint8_t priority)
{
  if (irq < 0)
    SCB->SHP[(irq & 0x0F) - 4] = priority;
  else
    NVIC->IP[irq] = priority;
}
/*----------------------------------------------------------------------------*/
uint8_t nvicGetPriority(irq_t irq)
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
