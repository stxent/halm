/*
 * platform/nxp/gptimer.h
 * Copyright (C) 2013 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef GPTIMER_H_
#define GPTIMER_H_
/*----------------------------------------------------------------------------*/
#include "gptimer_base.h"
/*----------------------------------------------------------------------------*/
extern const struct TimerClass * const GpTimer;
/*----------------------------------------------------------------------------*/
/** Symbolic names for two different types of timers. */
enum
{
  GPTIMER_CT16B0 = 0,
  GPTIMER_CT16B1,
  GPTIMER_CT32B0,
  GPTIMER_CT32B1
};
/*----------------------------------------------------------------------------*/
enum gpTimerEvent
{
  GPTIMER_MATCH_AUTO = 0,
  GPTIMER_MATCH0,
  GPTIMER_MATCH1,
  GPTIMER_MATCH2,
  GPTIMER_MATCH3,
  GPTIMER_EVENT_END
};
/*----------------------------------------------------------------------------*/
struct GpTimerConfig
{
  /**
   * Optional: timer frequency. When it is set to zero  an actual peripheral
   * frequency value is used. Option is not used in external clock mode.
   */
  uint32_t frequency;
  /** Optional: external clock input. */
  pin_t input;
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

  /* Actual timer frequency */
  uint32_t frequency;
  /* Match channel used for counter reset */
  uint8_t event;
};
/*----------------------------------------------------------------------------*/
#endif /* GPTIMER_H_ */
