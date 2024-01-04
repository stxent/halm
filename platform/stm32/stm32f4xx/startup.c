/*
 * startup.c
 * Copyright (C) 2024 xent
 * Project is distributed under the terms of the MIT License
 */

#ifdef CONFIG_CORE_CORTEX_FPU
#include <halm/core/cortex/fpu.h>
#endif
/*----------------------------------------------------------------------------*/
void platformStartup(void)
{
#ifdef CONFIG_CORE_CORTEX_FPU
  fpuEnable();
#endif
}
