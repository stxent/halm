/*
 * startup.c
 * Copyright (C) 2023 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/core/cortex/fpu.h>
#include <halm/core/cortex/mpu.h>
#include <halm/platform/numicro/system.h>
/*----------------------------------------------------------------------------*/
void platformStartup(void)
{
  sysResetBlock(RST_GPIO);

#ifdef CONFIG_CORE_CORTEX_FPU
  fpuEnable();
#endif

#ifdef CONFIG_CORE_CORTEX_MPU
  mpuEnable();
#endif
}
