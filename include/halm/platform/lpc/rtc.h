/*
 * halm/platform/lpc/rtc.h
 * Copyright (C) 2015 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_LPC_RTC_H_
#define HALM_PLATFORM_LPC_RTC_H_
/*----------------------------------------------------------------------------*/
#include <halm/platform/lpc/rtc_base.h>
/*----------------------------------------------------------------------------*/
extern const struct RtClockClass * const Rtc;

struct RtcConfig
{
  /** Optional: initial time. */
  time64_t timestamp;
  /** Optional: interrupt priority. */
  IrqPriority priority;
};

struct Rtc
{
  struct RtcBase base;

  void (*callback)(void *);
  void *callbackArgument;
};
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_LPC_RTC_H_ */
