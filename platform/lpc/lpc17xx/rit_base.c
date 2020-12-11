/*
 * rit_base.c
 * Copyright (C) 2018 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/platform/lpc/clocking.h>
#include <halm/platform/lpc/rit_base.h>
#include <halm/platform/lpc/system.h>
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
