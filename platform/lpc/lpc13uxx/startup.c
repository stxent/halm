/*
 * startup.c
 * Copyright (C) 2020 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/platform/lpc/lpc13uxx/system.h>
#include <halm/platform/lpc/lpc13uxx/system_defs.h>
#include <xcore/bits.h>
/*----------------------------------------------------------------------------*/
void platformStartup(void)
{
  sysClockDisable(CLK_I2C);

  /* Enable clock for IOCON block, clock for GPIO is enabled by default */
  sysClockEnable(CLK_IOCON);
}
