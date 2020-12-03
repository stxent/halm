/*
 * nvic.c
 * Copyright (C) 2017 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/core/core_defs.h>
#include <halm/core/cortex/armv7m/nvic_defs.h>
#include <halm/core/cortex/nvic.h>
#include <halm/platform/platform_defs.h>
#include <xcore/asm.h>
#include <assert.h>
/*----------------------------------------------------------------------------*/
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

  /* The reset sequence recommended by ARM application notes */
  __dsb();
  __interruptsDisable();

  /* Execute reset */
  SCB->AIRCR = value;

  /* Wait until reset */
  while (1);
}
/*----------------------------------------------------------------------------*/
void nvicSetVectorTableOffset(uint32_t offset)
{
  SCB->VTOR = offset;
}
