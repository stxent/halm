/*
 * nvic.c
 * Copyright (C) 2017 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <assert.h>
#include <xcore/bits.h>
#include <halm/core/cortex/nvic.h>
#include <halm/platform/platform_defs.h>
/*----------------------------------------------------------------------------*/
#define AIRCR_VECTRESET           BIT(0)
#define AIRCR_VECTCLRACTIVE       BIT(1)
#define AIRCR_SYSRESETREQ         BIT(2)

#define AIRCR_VECTKEY_MASK        BIT_FIELD(MASK(16), 16)
#define AIRCR_VECTKEY(value)      BIT_FIELD((value), 16)

#define AIRCR_PRIGROUP_MASK       BIT_FIELD(MASK(3), 8)
#define AIRCR_PRIGROUP(value)     BIT_FIELD((value), 8)
#define AIRCR_PRIGROUP_VALUE(reg) FIELD_VALUE((reg), AIRCR_PRIGROUP_MASK, 8)

#define GROUPS_TO_VALUE(groups)   ((7 - NVIC_PRIORITY_SIZE) + (groups))
#define VALUE_TO_GROUPS(value)    ((value) - (7 - NVIC_PRIORITY_SIZE))
/*----------------------------------------------------------------------------*/
uint8_t nvicGetPriorityGrouping(void)
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
/*----------------------------------------------------------------------------*/
void nvicResetCore(void)
{
  uint32_t value;

  value = SCB->AIRCR & ~AIRCR_VECTKEY_MASK;
  value |= AIRCR_VECTKEY(0x5FA) | AIRCR_SYSRESETREQ;
  SCB->AIRCR = value;
}
/*----------------------------------------------------------------------------*/
void nvicSetVectorTableOffset(uint32_t offset)
{
  SCB->VTOR = offset;
}
