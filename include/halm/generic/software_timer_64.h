/*
 * halm/generic/software_timer_64.h
 * Copyright (C) 2019 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_GENERIC_SOFTWARE_TIMER_64_H_
#define HALM_GENERIC_SOFTWARE_TIMER_64_H_
/*----------------------------------------------------------------------------*/
#include <halm/timer.h>
/*----------------------------------------------------------------------------*/
extern const struct Timer64Class * const SoftwareTimer64;

struct SoftwareTimer64Config
{
  /** Mandatory: base timer. */
  struct Timer *timer;
};

struct SoftwareTimer64
{
  struct Timer64 base;

  void (*callback)(void *);
  void *callbackArgument;

  struct Timer *timer;
  uint64_t ticks;
};
/*----------------------------------------------------------------------------*/
#endif /* HALM_GENERIC_SOFTWARE_TIMER_64_H_ */
