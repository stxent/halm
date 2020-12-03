/*
 * rit_base.c
 * Copyright (C) 2018 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <halm/platform/lpc/lpc17xx/clocking.h>
#include <halm/platform/lpc/lpc17xx/system.h>
#include <halm/platform/lpc/rit_base.h>
/*----------------------------------------------------------------------------*/
#define DEFAULT_DIV CLK_DIV1
/*----------------------------------------------------------------------------*/
uint32_t ritGetClock(void)
{
  return clockFrequency(MainClock) / sysClockDivToValue(DEFAULT_DIV);
}
/*----------------------------------------------------------------------------*/
void ritBaseInit(void)
{
  sysPowerEnable(PWR_RIT);
  sysClockControl(CLK_RIT, DEFAULT_DIV);
}
/*----------------------------------------------------------------------------*/
void ritBaseDeinit(void)
{
  sysPowerDisable(PWR_RIT);
}
