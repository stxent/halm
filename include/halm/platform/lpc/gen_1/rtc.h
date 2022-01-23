/*
 * halm/platform/lpc/gen_1/rtc.h
 * Copyright (C) 2022 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_LPC_RTC_H_
#error This header should not be included directly
#endif

#ifndef HALM_PLATFORM_LPC_GEN_1_RTC_H_
#define HALM_PLATFORM_LPC_GEN_1_RTC_H_
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
BEGIN_DECLS

int32_t rtcCalcCalibration(time64_t, time64_t, time64_t, int32_t);
void rtcSetCalibration(void *, int32_t);

END_DECLS
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_LPC_GEN_1_RTC_H_ */
