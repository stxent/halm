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
    .deinit = NULL, /* Default destructor */

    .setAlarm = NULL,
    .setCallback = NULL,
    .setTime = NULL,
    .stop = NULL,
    .time = clkTime
};
/*----------------------------------------------------------------------------*/
static enum Result clkInit(void *, const void *)
{
  return E_OK;
}
/*----------------------------------------------------------------------------*/
static time64_t clkTime(void *)
{
  return (time64_t)time(NULL);
}
