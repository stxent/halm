/*
 * nvic.c
 * Copyright (C) 2017 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/core/core_defs.h>
#include <halm/core/cortex/nvic.h>
#include <halm/core/cortex/scb_defs.h>
#include <xcore/asm.h>
/*----------------------------------------------------------------------------*/
void nvicResetCore(void)
{
  uint32_t value;

  value = SCB->AIRCR & ~AIRCR_VECTKEY_MASK;
  value |= AIRCR_VECTKEY(0x5FA) | AIRCR_SYSRESETREQ;

  /* The reset sequence recommended by ARM application notes */
  __dsb();
  __cpsid();

  /* Execute reset */
  SCB->AIRCR = value;

  /* Wait until reset */
  while (1);
}
