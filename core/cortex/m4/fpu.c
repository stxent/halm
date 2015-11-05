/*
 * fpu.c
 * Copyright (C) 2015 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <core/core_defs.h>
#include <core/cortex/m4/fpu_defs.h>
/*----------------------------------------------------------------------------*/
void fpuDisable()
{
  SCB->CPACR &= ~(CPACR_CP10_MASK | CPACR_CP11_MASK);
}
/*----------------------------------------------------------------------------*/
void fpuEnable()
{
  SCB->CPACR |= CPACR_CP10(CPACR_FULL_ACCESS) | CPACR_CP11(CPACR_FULL_ACCESS);
}
