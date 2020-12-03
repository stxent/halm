/*
 * halm/generic/software_timer_32.h
 * Copyright (C) 2019 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_GENERIC_SOFTWARE_TIMER_32_H_
#define HALM_GENERIC_SOFTWARE_TIMER_32_H_
/*----------------------------------------------------------------------------*/
#include <halm/timer.h>
/*----------------------------------------------------------------------------*/
extern const struct TimerClass * const SoftwareTimer32;

struct SoftwareTimer32Config
{
  /** Mandatory: base timer. */
  struct Timer *timer;
};

struct SoftwareTimer32
{
  struct Timer base;

  void (*callback)(void *);
  void *callbackArgument;

  struct Timer *timer;
  uint32_t ticks;
};
/*----------------------------------------------------------------------------*/
#endif /* HALM_GENERIC_SOFTWARE_TIMER_32_H_ */
