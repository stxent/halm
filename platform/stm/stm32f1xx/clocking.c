/*
 * clocking.c
 * Copyright (C) 2016 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <assert.h>
#include <stdbool.h>
#include <halm/clock.h>
#include <halm/platform/platform_defs.h>
/*----------------------------------------------------------------------------*/
#define HSI_OSC_FREQUENCY     8000000
#define TICK_RATE(frequency)  ((frequency) / 1000)
/*----------------------------------------------------------------------------*/
static void mainClockDisable(const void *);
static enum result mainClockEnable(const void *, const void *);
static uint32_t mainClockFrequency(const void *);
static bool mainClockReady(const void *);
/*----------------------------------------------------------------------------*/
static const struct ClockClass mainClockTable = {
    .disable = mainClockDisable,
    .enable = mainClockEnable,
    .frequency = mainClockFrequency,
    .ready = mainClockReady
};
/*----------------------------------------------------------------------------*/
const struct ClockClass * const MainClock = &mainClockTable;
/*----------------------------------------------------------------------------*/
uint32_t ticksPerSecond = TICK_RATE(HSI_OSC_FREQUENCY);
/*----------------------------------------------------------------------------*/
static void mainClockDisable(const void *clockBase __attribute__((unused)))
{

}
/*----------------------------------------------------------------------------*/
static enum result mainClockEnable(const void *clockBase
    __attribute__((unused)), const void *configBase __attribute__((unused)))
{
  return E_OK;
}
/*----------------------------------------------------------------------------*/
static uint32_t mainClockFrequency(const void *clockBase
    __attribute__((unused)))
{
  return HSI_OSC_FREQUENCY;
}
/*----------------------------------------------------------------------------*/
static bool mainClockReady(const void *clockBase __attribute__((unused)))
{
  return true;
}
