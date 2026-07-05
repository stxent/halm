/*
 * halm/platform/bouffalo/gptimer.h
 * Copyright (C) 2026 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_BOUFFALO_GPTIMER_H_
#define HALM_PLATFORM_BOUFFALO_GPTIMER_H_
/*----------------------------------------------------------------------------*/
#include <halm/platform/bouffalo/gptimer_base.h>
/*----------------------------------------------------------------------------*/
extern const struct TimerClass * const GpTimer;

struct GpTimerConfig
{
  /**
   * Optional: desired timer tick rate in Hertz. If this field is set to zero,
   * the actual peripheral frequency will be used as the timer tick rate.
   */
  uint32_t frequency;
  /** Optional: interrupt priority for the timer interrupt request. */
  IrqPriority priority;
  /** Mandatory: peripheral identifier number of the timer. */
  uint8_t channel;
};

struct GpTimer
{
  struct GpTimerBase base;

  void (*callback)(void *);
  void *callbackArgument;

  /* Desired timer frequency */
  uint32_t frequency;
};
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_BOUFFALO_GPTIMER_H_ */
