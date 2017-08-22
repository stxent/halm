/*
 * rtc.c
 * Copyright (C) 2017 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <time.h>
#include <halm/platform/linux/rtc.h>
/*----------------------------------------------------------------------------*/
static enum Result clkInit(void *, const void *);
static void clkDeinit(void *);
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
static const struct RtClockClass clkTable = {
    .size = sizeof(struct Rtc),
    .init = clkInit,
    .deinit = clkDeinit,

    .callback = clkCallback,
    .setAlarm = clkSetAlarm,
    .setTime = clkSetTime,
    .time = clkTime
};
/*----------------------------------------------------------------------------*/
const struct RtClockClass * const Rtc = &clkTable;
/*----------------------------------------------------------------------------*/
static enum Result clkInit(void *object __attribute__((unused)),
    const void *configBase __attribute__((unused)))
{
  return E_OK;
}
/*----------------------------------------------------------------------------*/
static void clkDeinit(void *object __attribute__((unused)))
{

}
/*----------------------------------------------------------------------------*/
static enum Result clkCallback(void *object __attribute__((unused)),
    void (*callback)(void *) __attribute__((unused)),
    void *argument __attribute__((unused)))
{
  return E_ERROR;
}
/*----------------------------------------------------------------------------*/
static enum Result clkSetAlarm(void *object __attribute__((unused)),
    time64_t alarmTime __attribute__((unused)))
{
  return E_ERROR;
}
/*----------------------------------------------------------------------------*/
static enum Result clkSetTime(void *object __attribute__((unused)),
    time64_t currentTime __attribute__((unused)))
{
  return E_ERROR;
}
/*----------------------------------------------------------------------------*/
static time64_t clkTime(void *object __attribute__((unused)))
{
  return (time64_t)time(0);
}
