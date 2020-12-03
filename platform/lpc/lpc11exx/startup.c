/*
 * startup.c
 * Copyright (C) 2015 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/platform/lpc/lpc11exx/system.h>
#include <halm/platform/lpc/lpc11exx/system_defs.h>
#include <xcore/bits.h>
/*----------------------------------------------------------------------------*/
void platformStartup(void)
{
  sysClockDisable(CLK_SSP0);
  sysClockDisable(CLK_USB);
  sysClockDisable(CLK_RAM2);

  /* Enable clock for IOCON block, clock for GPIO is enabled by default */
  sysClockEnable(CLK_IOCON);
}
