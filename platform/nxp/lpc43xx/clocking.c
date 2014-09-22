/*
 * clocking.c
 * Copyright (C) 2014 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <platform/platform_defs.h>
#include <platform/nxp/lpc43xx/clocking.h>
/*----------------------------------------------------------------------------*/
#define INT_OSC_FREQUENCY 12000000
/*----------------------------------------------------------------------------*/
static uint32_t mainClockFrequency(void);
/*----------------------------------------------------------------------------*/
static const struct ClockClass mainClockTable = {
    .disable = 0,
    .enable = 0,
    .frequency = mainClockFrequency,
    .ready = 0
};
/*----------------------------------------------------------------------------*/
const struct ClockClass * const MainClock = &mainClockTable;
/*----------------------------------------------------------------------------*/
uint32_t coreClock = INT_OSC_FREQUENCY;
/*----------------------------------------------------------------------------*/
static uint32_t mainClockFrequency(void)
{
  return coreClock;
}
