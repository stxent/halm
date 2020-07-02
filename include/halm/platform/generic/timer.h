/*
 * halm/platform/generic/timer.h
 * Copyright (C) 2020 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
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
   * Optional: desired timer tick rate. An actual peripheral frequency is used
   * when option is set to zero.
   */
  uint32_t frequency;
};
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_GENERIC_TIMER_H_ */
