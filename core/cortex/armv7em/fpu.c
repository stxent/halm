/*
 * fpu.c
 * Copyright (C) 2015 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/core/core_defs.h>
#include <halm/core/cortex/armv7em/fpu_defs.h>
/*----------------------------------------------------------------------------*/
void fpuDisable(void)
{
  SCB->CPACR &= ~(CPACR_CP10_MASK | CPACR_CP11_MASK);
}
/*----------------------------------------------------------------------------*/
void fpuEnable(void)
{
  SCB->CPACR |= CPACR_CP10(CPACR_FULL_ACCESS) | CPACR_CP11(CPACR_FULL_ACCESS);
}
