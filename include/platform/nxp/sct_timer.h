/*
 * platform/nxp/sct_timer.h
 * Copyright (C) 2014 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef PLATFORM_NXP_SCT_TIMER_H_
#define PLATFORM_NXP_SCT_TIMER_H_
/*----------------------------------------------------------------------------*/
#include <platform/nxp/sct_base.h>
/*----------------------------------------------------------------------------*/
extern const struct TimerClass * const SctTimer;
/*----------------------------------------------------------------------------*/
struct SctTimerConfig
{
  /**
   * Optional: desired timer tick rate. An actual peripheral frequency is used
   * when option is set to zero.
   */
  uint32_t frequency;
  /** Optional: timer interrupt priority. */
  irqPriority priority;
  /** Mandatory: peripheral identifier. */
  uint8_t channel;
  /** Optional: do not enable the timer after the initialization. */
  bool disabled;
  /** Optional: timer part. */
  enum sctPart part;
};
/*----------------------------------------------------------------------------*/
struct SctTimer
{
  struct SctBase base;

  void (*callback)(void *);
  void *callbackArgument;

  /* Desired timer frequency */
  uint32_t frequency;
  /* Match channel used for counter reset */
  uint8_t event;
};
/*----------------------------------------------------------------------------*/
#endif /* PLATFORM_NXP_SCT_TIMER_H_ */
