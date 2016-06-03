/*
 * startup.c
 * Copyright (C) 2015 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <xcore/bits.h>
#include <halm/platform/nxp/lpc11exx/system.h>
#include <halm/platform/nxp/lpc11exx/system_defs.h>
/*----------------------------------------------------------------------------*/
void platformStartup(void)
{
  sysClockDisable(CLK_SSP0);
  sysClockDisable(CLK_USB);
  sysClockDisable(CLK_RAM2);

  /* Enable clock for IOCON block, clock for GPIO is enabled by default */
  sysClockEnable(CLK_IOCON);
}
