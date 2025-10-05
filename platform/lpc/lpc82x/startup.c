/*
 * startup.c
 * Copyright (C) 2025 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/platform/lpc/lpc82x/system_defs.h>
#include <halm/platform/lpc/system.h>
/*----------------------------------------------------------------------------*/
void platformStartup(void)
{
  /* Clocks for GPIO and SWM are enabled by default */

  /* Enable clock for IOCON block */
  sysClockEnable(CLK_IOCON);
}
