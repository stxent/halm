/*
 * halm/platform/nxp/gptimer_counter.h
 * Copyright (C) 2016 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef HALM_PLATFORM_NXP_GPTIMER_COUNTER_H_
#define HALM_PLATFORM_NXP_GPTIMER_COUNTER_H_
/*----------------------------------------------------------------------------*/
#include <halm/capture.h>
#include <halm/platform/nxp/gptimer_base.h>
/*----------------------------------------------------------------------------*/
extern const struct TimerClass * const GpTimerCounter;
/*----------------------------------------------------------------------------*/
struct GpTimerCounterConfig
{
  /** Mandatory: active edge. */
  enum pinEvent edge;
  /** Mandatory: pin used as an input. */
  PinNumber pin;
  /** Optional: timer interrupt priority. */
  IrqPriority priority;
  /** Mandatory: peripheral identifier. */
  uint8_t channel;
};
/*----------------------------------------------------------------------------*/
struct GpTimerCounter
{
  struct GpTimerBase base;

  void (*callback)(void *);
  void *callbackArgument;

  /* Match channel used for counter reset */
  uint8_t event;
};
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_NXP_GPTIMER_COUNTER_H_ */
