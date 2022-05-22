/*
 * rtc.c
 * Copyright (C) 2017 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/platform/generic/rtc.h>
#include <time.h>
/*----------------------------------------------------------------------------*/
static enum Result clkInit(void *, const void *);
static time64_t clkTime(void *);
/*----------------------------------------------------------------------------*/
struct Rtc
{
  struct RtClock base;
};
/*----------------------------------------------------------------------------*/
const struct RtClockClass * const Rtc = &(const struct RtClockClass){
    .size = sizeof(struct Rtc),
    .init = clkInit,
    .deinit = 0, /* Default destructor */

    .setAlarm = 0,
    .setCallback = 0,
    .setTime = 0,
    .stop = 0,
    .time = clkTime
};
/*----------------------------------------------------------------------------*/
static enum Result clkInit(void *object __attribute__((unused)),
    const void *configBase __attribute__((unused)))
{
  return E_OK;
}
/*----------------------------------------------------------------------------*/
static time64_t clkTime(void *object __attribute__((unused)))
{
  return (time64_t)time(0);
}
