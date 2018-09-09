/*
 * startup.c
 * Copyright (C) 2016 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <stddef.h>
#include <halm/platform/stm/stm32f1xx/system.h>
/*----------------------------------------------------------------------------*/
void platformStartup(void)
{
  static const enum SysClockBranch clocksToEnable[] = {
      CLK_AFIO
  };

  for (size_t index = 0; index < ARRAY_SIZE(clocksToEnable); ++index)
    sysClockEnable(clocksToEnable[index]);
}
