/*
 * irq.c
 * Copyright (C) 2014 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <irq.h>
#include <platform/platform_defs.h>
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
