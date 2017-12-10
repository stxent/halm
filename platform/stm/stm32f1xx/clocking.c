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
static uint32_t mainClockFrequency(const void *);
static uint32_t apb1ClockFrequency(const void *);
static uint32_t apb2ClockFrequency(const void *);

static void clockDisableStub(const void *);
static enum Result clockEnableStub(const void *, const void *);
static bool clockReadyStub(const void *);
/*----------------------------------------------------------------------------*/
static const struct ClockClass mainClockTable = {
    .disable = clockDisableStub,
    .enable = clockEnableStub,
    .frequency = mainClockFrequency,
    .ready = clockReadyStub
};

static const struct ClockClass apb1ClockTable = {
    .disable = clockDisableStub,
    .enable = clockEnableStub,
    .frequency = apb1ClockFrequency,
    .ready = clockReadyStub
};

static const struct ClockClass apb2ClockTable = {
    .disable = clockDisableStub,
    .enable = clockEnableStub,
    .frequency = apb2ClockFrequency,
    .ready = clockReadyStub
};
/*----------------------------------------------------------------------------*/
const struct ClockClass * const MainClock = &mainClockTable;
const struct ClockClass * const Apb1Clock = &apb1ClockTable;
const struct ClockClass * const Apb2Clock = &apb2ClockTable;
/*----------------------------------------------------------------------------*/
uint32_t ticksPerSecond = TICK_RATE(HSI_OSC_FREQUENCY);
/*----------------------------------------------------------------------------*/
static uint32_t mainClockFrequency(const void *clockBase
    __attribute__((unused)))
{
  return HSI_OSC_FREQUENCY;
}
/*----------------------------------------------------------------------------*/
static uint32_t apb1ClockFrequency(const void *clockBase
    __attribute__((unused)))
{
  return HSI_OSC_FREQUENCY;
}
/*----------------------------------------------------------------------------*/
static uint32_t apb2ClockFrequency(const void *clockBase
    __attribute__((unused)))
{
  return HSI_OSC_FREQUENCY;
}
/*----------------------------------------------------------------------------*/
static void clockDisableStub(const void *clockBase __attribute__((unused)))
{
}
/*----------------------------------------------------------------------------*/
static enum Result clockEnableStub(const void *clockBase
    __attribute__((unused)), const void *configBase __attribute__((unused)))
{
  return E_OK;
}
/*----------------------------------------------------------------------------*/
static bool clockReadyStub(const void *clockBase __attribute__((unused)))
{
  return true;
}
