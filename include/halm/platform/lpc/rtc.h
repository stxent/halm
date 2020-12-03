/*
 * halm/platform/lpc/rtc.h
 * Copyright (C) 2015 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_LPC_RTC_H_
#define HALM_PLATFORM_LPC_RTC_H_
/*----------------------------------------------------------------------------*/
#include <halm/target.h>
/*----------------------------------------------------------------------------*/
#undef HEADER_PATH
#define HEADER_PATH <halm/platform/PLATFORM_TYPE/GEN_RTC/rtc_base.h>
#include HEADER_PATH
#undef HEADER_PATH
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
