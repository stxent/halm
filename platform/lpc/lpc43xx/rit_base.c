/*
 * rit_base.c
 * Copyright (C) 2018 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/platform/lpc/clocking.h>
#include <halm/platform/lpc/rit_base.h>
#include <halm/platform/lpc/system.h>
/*----------------------------------------------------------------------------*/
uint32_t ritGetClock(void)
{
  return clockFrequency(MainClock);
}
/*----------------------------------------------------------------------------*/
void ritBaseInit(void)
{
  sysClockEnable(CLK_M4_RIT);
  sysResetEnable(RST_RIT);
}
/*----------------------------------------------------------------------------*/
void ritBaseDeinit(void)
{
  sysClockDisable(CLK_M4_RIT);
}
