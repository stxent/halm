/*
 * platform/nxp/rtc.h
 * Copyright (C) 2015 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef PLATFORM_NXP_RTC_H_
#define PLATFORM_NXP_RTC_H_
/*----------------------------------------------------------------------------*/
#include <libhalm/target.h>
/*----------------------------------------------------------------------------*/
#undef HEADER_PATH
#define HEADER_PATH <platform/PLATFORM_TYPE/GEN_RTC/rtc_base.h>
#include HEADER_PATH
#undef HEADER_PATH
/*----------------------------------------------------------------------------*/
extern const struct RtClockClass * const Rtc;
/*----------------------------------------------------------------------------*/
struct RtcConfig
{
  /** Optional: initial time. */
  time64_t timestamp;
  /** Optional: interrupt priority. */
  irqPriority priority;
};
/*----------------------------------------------------------------------------*/
struct Rtc
{
  struct RtcBase parent;

  void (*callback)(void *);
  void *callbackArgument;
};
/*----------------------------------------------------------------------------*/
#endif /* PLATFORM_NXP_RTC_H_ */
