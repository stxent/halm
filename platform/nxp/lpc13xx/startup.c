/*
 * startup.c
 * Copyright (C) 2015 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <bits.h>
#include <platform/nxp/lpc13xx/system.h>
#include <platform/nxp/lpc13xx/system_defs.h>
/*----------------------------------------------------------------------------*/
void platformStartup(void)
{
  sysClockDisable(CLK_SSP0);
  sysClockDisable(CLK_USBREG);

  /* Enable clock for IOCON block, clock for GPIO is enabled by default */
  sysClockEnable(CLK_IOCON);
}
