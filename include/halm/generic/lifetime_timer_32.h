/*
 * halm/generic/lifetime_timer_32.h
 * Copyright (C) 2019 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_GENERIC_LIFETIME_TIMER_32_H_
#define HALM_GENERIC_LIFETIME_TIMER_32_H_
/*----------------------------------------------------------------------------*/
#include <halm/timer.h>
/*----------------------------------------------------------------------------*/
extern const struct TimerClass * const LifetimeTimer32;

struct LifetimeTimer32Config
{
  /** Mandatory: base timer. */
  struct Timer *timer;
};

struct LifetimeTimer32
{
  struct Timer base;

  struct Timer *timer;
  uint32_t ticks;
};
/*----------------------------------------------------------------------------*/
#endif /* HALM_GENERIC_LIFETIME_TIMER_32_H_ */
