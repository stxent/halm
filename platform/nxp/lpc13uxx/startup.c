/*
 * startup.c
 * Copyright (C) 2020 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <xcore/bits.h>
#include <halm/platform/nxp/lpc13uxx/system.h>
#include <halm/platform/nxp/lpc13uxx/system_defs.h>
/*----------------------------------------------------------------------------*/
void platformStartup(void)
{
  sysClockDisable(CLK_I2C);

  /* Enable clock for IOCON block, clock for GPIO is enabled by default */
  sysClockEnable(CLK_IOCON);
}
