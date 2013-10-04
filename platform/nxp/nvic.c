/*
 * nvic.c
 * Copyright (C) 2013 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include "platform/nxp/nvic.h"
/*----------------------------------------------------------------------------*/
#define AIRCR_VECTKEY_MASK              0xFFFF0000UL
#define AIRCR_VECTKEY(value)            ((unsigned long)(value) << 16)
#define AIRCR_PRIGROUP_MASK             0x00000700UL
#define AIRCR_PRIGROUP(value)           ((unsigned long)(value) << 8)
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
  uint32_t value;

  /* Argument is the number of subpriority bits */
  if (subpriority > 7)
    return;

  value = SCB->AIRCR & ~(AIRCR_VECTKEY_MASK | AIRCR_PRIGROUP_MASK);
  value |= AIRCR_VECTKEY(0x5FA) | AIRCR_PRIGROUP(subpriority);
  SCB->AIRCR = value;
}
