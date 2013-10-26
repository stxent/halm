/*
 * irq.c
 * Copyright (C) 2013 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <irq.h>
/*----------------------------------------------------------------------------*/
#define HEADER_PATH <platform/PLATFORM_TYPE/platform_defs.h>
#include HEADER_PATH
/*----------------------------------------------------------------------------*/
#define AIRCR_VECTKEY_MASK              0xFFFF0000UL
#define AIRCR_VECTKEY(value)            ((unsigned long)(value) << 16)
#define AIRCR_PRIGROUP_MASK             0x00000700UL
#define AIRCR_PRIGROUP(value)           ((unsigned long)(value) << 8)
/*----------------------------------------------------------------------------*/
#define PRIORITY_TO_VALUE(priority) \
    ((((1 << NVIC_PRIORITY_SIZE) - 1) - (priority)) << (8 - NVIC_PRIORITY_SIZE))
#define VALUE_TO_PRIORITY(value) \
    ((((1 << NVIC_PRIORITY_SIZE) - 1) - (value)) << (8 - NVIC_PRIORITY_SIZE))
/*----------------------------------------------------------------------------*/
void irqSetPriority(irq_t irq, priority_t priority)
{
  if (irq < 0)
    SCB->SHP[(irq & 0x0F) - 4] = PRIORITY_TO_VALUE(priority) & 0xFF;
  else
    NVIC->IP[irq] = PRIORITY_TO_VALUE(priority) & 0xFF;
}
/*----------------------------------------------------------------------------*/
priority_t irqGetPriority(irq_t irq)
{
  if (irq < 0)
    return VALUE_TO_PRIORITY(SCB->SHP[(irq & 0x0F) - 4]);
  else
    return VALUE_TO_PRIORITY(NVIC->IP[irq]);
}
/*----------------------------------------------------------------------------*/
//void nvicSetPriorityGrouping(uint8_t subpriority)
//{
//  uint32_t value;
//
//  /* Argument is the number of subpriority bits */
//  if (subpriority > 7)
//    return;
//
//  value = SCB->AIRCR & ~(AIRCR_VECTKEY_MASK | AIRCR_PRIGROUP_MASK);
//  value |= AIRCR_VECTKEY(0x5FA) | AIRCR_PRIGROUP(subpriority);
//  SCB->AIRCR = value;
//}
