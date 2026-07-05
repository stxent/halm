/*
 * halm/generic/timer_factory.h
 * Copyright (C) 2016, 2023 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_GENERIC_TIMER_FACTORY_H_
#define HALM_GENERIC_TIMER_FACTORY_H_
/*----------------------------------------------------------------------------*/
#include <halm/timer.h>
/*----------------------------------------------------------------------------*/
extern const struct TimerClass * const TicklessFactory;
extern const struct TimerClass * const TimerFactory;

struct TimerFactoryEntry;

struct TimerFactoryConfig
{
  /**
   * Mandatory: timer for interrupt generation on a regular basis. The timer
   * period determines a minimum interval between software timer ticks.
   */
  struct Timer *timer;
};

struct TimerFactory
{
  struct Entity base;

  struct TimerFactoryEntry *head;
  struct Timer *timer;

  uint32_t counter;
  uint32_t overflow;
};
/*----------------------------------------------------------------------------*/
BEGIN_DECLS

void *timerFactoryCreate(void *);

END_DECLS
/*----------------------------------------------------------------------------*/
#endif /* HALM_GENERIC_TIMER_FACTORY_H_ */
