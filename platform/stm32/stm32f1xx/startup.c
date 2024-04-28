/*
 * startup.c
 * Copyright (C) 2016 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/core/core_defs.h>
#include <halm/core/cortex/mpu.h>
#include <halm/core/cortex/scb_defs.h>
#include <halm/platform/stm32/system.h>
#include <stddef.h>
/*----------------------------------------------------------------------------*/
void platformStartup(void)
{
  static const enum SysClockBranch clocksToEnable[] = {
      CLK_AFIO
  };

  /* Enable 64-bit alignment of stack pointer on interrupts */
  SCB->CCR |= CCR_STKALIGN;

  for (size_t index = 0; index < ARRAY_SIZE(clocksToEnable); ++index)
    sysClockEnable(clocksToEnable[index]);

#ifdef CONFIG_CORE_CORTEX_MPU
  mpuEnable();
#endif
}
