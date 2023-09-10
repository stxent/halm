/*
 * startup.c
 * Copyright (C) 2023 xent
 * Project is distributed under the terms of the MIT License
 */

#ifdef CONFIG_CORE_CORTEX_FPU
#include <halm/core/cortex/fpu.h>
#endif

#include <halm/platform/numicro/system.h>
/*----------------------------------------------------------------------------*/
void platformStartup(void)
{
  sysResetBlock(RST_GPIO);

#ifdef CONFIG_CORE_CORTEX_FPU
  fpuEnable();
#endif
}
