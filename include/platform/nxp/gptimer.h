/*
 * platform/nxp/gptimer.h
 * Copyright (C) 2013 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef PLATFORM_NXP_GPTIMER_H_
#define PLATFORM_NXP_GPTIMER_H_
/*----------------------------------------------------------------------------*/
#include <platform/nxp/gptimer_base.h>
/*----------------------------------------------------------------------------*/
extern const struct TimerClass * const GpTimer;
/*----------------------------------------------------------------------------*/
struct GpTimerConfig
{
  /**
   * Optional: desired timer tick rate. An actual peripheral frequency is used
   * when option is set to zero.
   */
  uint32_t frequency;
  /** Optional: timer interrupt priority. */
  priority_t priority;
  /** Mandatory: peripheral identifier. */
  uint8_t channel;
  /** Optional: do not enable the timer after the initialization. */
  bool disabled;
  /** Optional: match event used as a reset source for the timer. */
  enum gpTimerEvent event;
};
/*----------------------------------------------------------------------------*/
struct GpTimer
{
  struct GpTimerBase parent;

  void (*callback)(void *);
  void *callbackArgument;

  /* Desired timer frequency */
  uint32_t frequency;
  /* Match channel used for counter reset */
  uint8_t event;
};
/*----------------------------------------------------------------------------*/
#endif /* PLATFORM_NXP_GPTIMER_H_ */
