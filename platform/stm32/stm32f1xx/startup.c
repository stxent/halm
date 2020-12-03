/*
 * startup.c
 * Copyright (C) 2016 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/platform/stm32/stm32f1xx/system.h>
#include <stddef.h>
/*----------------------------------------------------------------------------*/
void platformStartup(void)
{
  static const enum SysClockBranch clocksToEnable[] = {
      CLK_AFIO
  };

  for (size_t index = 0; index < ARRAY_SIZE(clocksToEnable); ++index)
    sysClockEnable(clocksToEnable[index]);
}
