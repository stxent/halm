/*
 * halm/platform/imxrt/pit.h
 * Copyright (C) 2024 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_IMXRT_PIT_H_
#define HALM_PLATFORM_IMXRT_PIT_H_
/*----------------------------------------------------------------------------*/
#include <halm/platform/imxrt/pit_base.h>
/*----------------------------------------------------------------------------*/
/* Basic 32-bit version requires PitConfig configuration structure */
extern const struct TimerClass * const Pit;
/* Lifetime 64-bit version does not require configuration structure */
extern const struct Timer64Class * const Pit64;

struct PitConfig
{
  /**
   * Optional: desired timer tick rate in Hertz. Used in chained mode only.
   * If this field is set to zero, the actual peripheral frequency will be used
   * as the timer tick rate.
   */
  uint32_t frequency;
  /** Optional: interrupt priority for the timer interrupt request. */
  IrqPriority priority;
  /** Mandatory: peripheral identifier number of the timer. */
  uint8_t channel;
  /** Optional: enable chained mode to make a frequency divider. */
  bool chain;
};

struct Pit
{
  struct PitBase base;

  /* User interrupt handler */
  void (*callback)(void *);
  /* Argument passed to the user interrupt handler */
  void *callbackArgument;
};
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_IMXRT_PIT_H_ */
