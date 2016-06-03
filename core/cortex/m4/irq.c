/*
 * irq.c
 * Copyright (C) 2014 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <assert.h>
#include <xcore/bits.h>
#include <halm/irq.h>
#include <halm/platform/platform_defs.h>
/*----------------------------------------------------------------------------*/
#define AIRCR_VECTKEY_MASK        BIT_FIELD(MASK(16), 16)
#define AIRCR_VECTKEY(value)      BIT_FIELD((value), 16)
#define AIRCR_PRIGROUP_MASK       BIT_FIELD(MASK(3), 8)
#define AIRCR_PRIGROUP(value)     BIT_FIELD((value), 8)
#define AIRCR_PRIGROUP_VALUE(reg) FIELD_VALUE((reg), AIRCR_PRIGROUP_MASK, 8)

#define GROUPS_TO_VALUE(groups)   ((7 - NVIC_PRIORITY_SIZE) + (groups))
#define VALUE_TO_GROUPS(value)    ((value) - (7 - NVIC_PRIORITY_SIZE))

#define PRIORITY_TO_VALUE(priority) \
    ((((1 << NVIC_PRIORITY_SIZE) - 1) - (priority)) << (8 - NVIC_PRIORITY_SIZE))
#define VALUE_TO_PRIORITY(value) \
    (((1 << NVIC_PRIORITY_SIZE) - 1) - ((value) >> (8 - NVIC_PRIORITY_SIZE)))
/*----------------------------------------------------------------------------*/
void irqSetPriority(irqNumber irq, irqPriority priority)
{
  assert(priority < (1 << NVIC_PRIORITY_SIZE));

  const uint32_t value = PRIORITY_TO_VALUE(priority) & 0xFF;

  if (irq < 0)
    SCB->SHP[(irq & 0x0F) - 4] = value;
  else
    NVIC->IP[irq] = value;
}
/*----------------------------------------------------------------------------*/
irqPriority irqGetPriority(irqNumber irq)
{
  if (irq < 0)
    return VALUE_TO_PRIORITY(SCB->SHP[(irq & 0x0F) - 4]);
  else
    return VALUE_TO_PRIORITY(NVIC->IP[irq]);
}
/*----------------------------------------------------------------------------*/
uint8_t nvicGetPriorityGrouping()
{
  const int8_t groupBits = VALUE_TO_GROUPS(AIRCR_PRIGROUP_VALUE(SCB->AIRCR));

  return groupBits < 0 ? 0 : groupBits;
}
/*----------------------------------------------------------------------------*/
void nvicSetPriorityGrouping(uint8_t groupBits)
{
  assert(groupBits < 7 && groupBits <= NVIC_PRIORITY_SIZE);

  uint32_t value;

  value = SCB->AIRCR & ~(AIRCR_VECTKEY_MASK | AIRCR_PRIGROUP_MASK);
  value |= AIRCR_VECTKEY(0x5FA) | AIRCR_PRIGROUP(GROUPS_TO_VALUE(groupBits));
  SCB->AIRCR = value;
}
