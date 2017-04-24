/*
 * irq.c
 * Copyright (C) 2014 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <assert.h>
#include <halm/irq.h>
#include <halm/platform/platform_defs.h>
/*----------------------------------------------------------------------------*/
#define PRIORITY_TO_VALUE(priority) \
    ((((1 << NVIC_PRIORITY_SIZE) - 1) - (priority)) << (8 - NVIC_PRIORITY_SIZE))
#define VALUE_TO_PRIORITY(value) \
    (((1 << NVIC_PRIORITY_SIZE) - 1) - ((value) >> (8 - NVIC_PRIORITY_SIZE)))
/*----------------------------------------------------------------------------*/
void irqSetPriority(IrqNumber irq, IrqPriority priority)
{
  assert(priority < (1 << NVIC_PRIORITY_SIZE));

  const uint32_t value = PRIORITY_TO_VALUE(priority) & 0xFF;

  if (irq < 0)
    SCB->SHP[(irq & 0x0F) - 4] = value;
  else
    NVIC->IP[irq] = value;
}
/*----------------------------------------------------------------------------*/
IrqPriority irqGetPriority(IrqNumber irq)
{
  if (irq < 0)
    return VALUE_TO_PRIORITY(SCB->SHP[(irq & 0x0F) - 4]);
  else
    return VALUE_TO_PRIORITY(NVIC->IP[irq]);
}
