/*
 * irq.c
 * Copyright (C) 2014 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/irq.h>
#include <assert.h>
/*----------------------------------------------------------------------------*/
void irqSetPriority(IrqNumber irq, IrqPriority priority)
{
  assert(priority < (1 << NVIC_IRQ_BITS));

  const uint32_t value = IRQ_PRIORITY_TO_REG(priority) & 0xFF;

  if (irq < 0)
    SCB->SHP[(irq & 0x0F) - 4] = value;
  else
    NVIC->IPR[irq] = value;
}
/*----------------------------------------------------------------------------*/
IrqPriority irqGetPriority(IrqNumber irq)
{
  if (irq < 0)
    return IRQ_REG_TO_PRIORITY(SCB->SHP[(irq & 0x0F) - 4]);
  else
    return IRQ_REG_TO_PRIORITY(NVIC->IPR[irq]);
}
