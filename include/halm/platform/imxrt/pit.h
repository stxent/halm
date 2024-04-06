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
  /** Optional: timer frequency in chained mode. */
  uint32_t frequency;
  /** Optional: timer interrupt priority. */
  IrqPriority priority;
  /** Mandatory: peripheral identifier. */
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
