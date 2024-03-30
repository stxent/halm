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
extern const struct TimerClass * const Pit;
extern const struct Timer64Class * const Pit64; // TODO

struct PitConfig
{
  /** Optional: timer interrupt priority. */
  IrqPriority priority;
  /** Mandatory: peripheral identifier. */
  uint8_t channel;
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
