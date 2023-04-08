/*
 * irq.c
 * Copyright (C) 2013 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/irq.h>
#include <xcore/bits.h>
#include <assert.h>
/*----------------------------------------------------------------------------*/
#define IRQ_SHIFT(value)    (((uint32_t)(value) & 0x03) << 3)
#define CORE_OFFSET(value)  ((((uint32_t)(value) & 0x0F) - 8) >> 2)
/*----------------------------------------------------------------------------*/
void irqSetPriority(IrqNumber irq, IrqPriority priority)
{
  assert(priority < (1 << NVIC_IRQ_BITS));

  const uint32_t shift = IRQ_SHIFT(irq);
  const uint32_t mask = ~BIT_FIELD(MASK(8), shift);
  const uint32_t value = IRQ_PRIORITY_TO_REG(priority) << shift;

  if (irq < 0)
  {
    const uint32_t offset = CORE_OFFSET(irq);

    SCB->SHP[offset] = (SCB->SHP[offset] & mask) | value;
  }
  else
  {
    const uint32_t offset = irq >> 2;

    NVIC->IPR[offset] = (NVIC->IPR[offset] & mask) | value;
  }
}
/*----------------------------------------------------------------------------*/
IrqPriority irqGetPriority(IrqNumber irq)
{
  const uint32_t shift = IRQ_SHIFT(irq);

  if (irq < 0)
    return IRQ_REG_TO_PRIORITY(SCB->SHP[CORE_OFFSET(irq)] >> shift);
  else
    return IRQ_REG_TO_PRIORITY(NVIC->IPR[irq >> 2] >> shift);
}
