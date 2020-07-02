/*
 * rit_base.c
 * Copyright (C) 2020 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <halm/platform/nxp/lpc13uxx/clocking.h>
#include <halm/platform/nxp/rit_base.h>
/*----------------------------------------------------------------------------*/
uint32_t ritGetClock(void)
{
  return clockFrequency(MainClock);
}
/*----------------------------------------------------------------------------*/
void ritBaseInit(void)
{
}
/*----------------------------------------------------------------------------*/
void ritBaseDeinit(void)
{
}
