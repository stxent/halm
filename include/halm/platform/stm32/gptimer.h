/*
 * halm/platform/stm32/gptimer.h
 * Copyright (C) 2019 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_STM32_GPTIMER_H_
#define HALM_PLATFORM_STM32_GPTIMER_H_
/*----------------------------------------------------------------------------*/
#include <halm/platform/stm32/gptimer_base.h>
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
  /** Optional: event used for DMA request and internal request generation. */
  enum GpTimerEvent event;
  /** Mandatory: peripheral identifier number of the timer. */
  uint8_t channel;
  /**
   * Optional: flag to disable automatic timer reset on overflow
   * or match events. When set to @b true, enables free-running mode where
   * the timer counter continues incrementing without being automatically
   * reset by overflow or compare events.
   */
  bool freerun;
};

struct GpTimer
{
  struct GpTimerBase base;

  void (*callback)(void *);
  void *callbackArgument;

  /* Desired timer frequency */
  uint32_t frequency;
  /* Event used for DMA request generation */
  enum GpTimerEvent event;
  /* Enable free-running mode */
  bool freerun;
};
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_STM32_GPTIMER_H_ */
