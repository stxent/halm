/*
 * pm.c
 * Copyright (C) 2026 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/pm.h>
#include <xcore/asm.h>
/*----------------------------------------------------------------------------*/
void pmCoreChangeState(enum PmState);
/*----------------------------------------------------------------------------*/
void pmCoreChangeState(enum PmState state)
{
  if (state != PM_ACTIVE)
  {
    __fence();
    __wfi();
  }
}
