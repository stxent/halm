/*
 * startup.c
 * Copyright (C) 2024 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/core/cortex/fpu.h>
#include <halm/core/cortex/mpu.h>
/*----------------------------------------------------------------------------*/
void platformStartup(void)
{
#ifdef CONFIG_CORE_CORTEX_FPU
  fpuEnable();
#endif

#ifdef CONFIG_CORE_CORTEX_MPU
  mpuEnable();
#endif
}
