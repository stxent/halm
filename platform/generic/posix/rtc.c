/*
 * rtc.c
 * Copyright (C) 2017 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <time.h>
#include <halm/platform/generic/rtc.h>
/*----------------------------------------------------------------------------*/
static enum Result clkInit(void *, const void *);
static enum Result clkCallback(void *, void (*)(void *), void *);
static enum Result clkSetAlarm(void *, time64_t);
static enum Result clkSetTime(void *, time64_t);
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

    .callback = clkCallback,
    .setAlarm = clkSetAlarm,
    .setTime = clkSetTime,
    .time = clkTime
};
/*----------------------------------------------------------------------------*/
static enum Result clkInit(void *object __attribute__((unused)),
    const void *configBase __attribute__((unused)))
{
  return E_OK;
}
/*----------------------------------------------------------------------------*/
static enum Result clkCallback(void *object __attribute__((unused)),
    void (*callback)(void *) __attribute__((unused)),
    void *argument __attribute__((unused)))
{
  return E_INVALID;
}
/*----------------------------------------------------------------------------*/
static enum Result clkSetAlarm(void *object __attribute__((unused)),
    time64_t alarmTime __attribute__((unused)))
{
  return E_INVALID;
}
/*----------------------------------------------------------------------------*/
static enum Result clkSetTime(void *object __attribute__((unused)),
    time64_t currentTime __attribute__((unused)))
{
  return E_INVALID;
}
/*----------------------------------------------------------------------------*/
static time64_t clkTime(void *object __attribute__((unused)))
{
  return (time64_t)time(0);
}
