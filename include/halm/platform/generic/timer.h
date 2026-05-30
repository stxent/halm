/*
 * halm/platform/generic/timer.h
 * Copyright (C) 2020 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_GENERIC_TIMER_H_
#define HALM_PLATFORM_GENERIC_TIMER_H_
/*----------------------------------------------------------------------------*/
#include <halm/timer.h>
/*----------------------------------------------------------------------------*/
extern const struct TimerClass * const Timer;

struct TimerConfig
{
  /**
   * Optional: desired timer tick rate in Hz. When this value is zero,
   * the system uses a default frequency of 1 kHz, which matches
   * the fundamental system timer granularity (1 millisecond).
   * The frequency cannot exceed 1 kHz.
   */
  uint32_t frequency;
};
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_GENERIC_TIMER_H_ */
