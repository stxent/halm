/*
 * irq.c
 * Copyright (C) 2013 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <irq.h>
/*----------------------------------------------------------------------------*/
#define IRQ_SHIFT(value)    (((uint32_t)(value) & 0x03) << 3)
#define CORE_OFFSET(value)  ((((uint32_t)(value) & 0x0F) - 8) >> 2)
/*----------------------------------------------------------------------------*/
void irqSetPriority(irqNumber irq, irqPriority priority)
{
  if (irq < 0)
  {
    SCB->SHP[CORE_OFFSET(irq)] = (SCB->SHP[CORE_OFFSET(irq)]
        & ~(0xFF << IRQ_SHIFT(irq))) | ((priority) << IRQ_SHIFT(irq));
  }
  else
  {
    NVIC->IPR[irq >> 2] = (NVIC->IPR[irq >> 2] & ~(0xFF << IRQ_SHIFT(irq)))
        | ((priority) << IRQ_SHIFT(irq));
  }
}
/*----------------------------------------------------------------------------*/
irqPriority irqGetPriority(irqNumber irq)
{
  if (irq < 0)
    return SCB->SHP[CORE_OFFSET(irq)] >> IRQ_SHIFT(irq);
  else
    return NVIC->IPR[irq >> 2] >> IRQ_SHIFT(irq);
}
