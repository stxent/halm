/*
 * nvic.c
 * Copyright (C) 2017 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <xcore/bits.h>
#include <halm/core/cortex/nvic.h>
#include <halm/platform/platform_defs.h>
/*----------------------------------------------------------------------------*/
#define AIRCR_VECTCLRACTIVE       BIT(1)
#define AIRCR_SYSRESETREQ         BIT(2)

#define AIRCR_VECTKEY_MASK        BIT_FIELD(MASK(16), 16)
#define AIRCR_VECTKEY(value)      BIT_FIELD((value), 16)
/*----------------------------------------------------------------------------*/
void nvicResetCore(void)
{
  uint32_t value;

  value = SCB->AIRCR & ~AIRCR_VECTKEY_MASK;
  value |= AIRCR_VECTKEY(0x5FA) | AIRCR_SYSRESETREQ;
  SCB->AIRCR = value;
}
