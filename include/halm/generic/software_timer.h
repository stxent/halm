/*
 * halm/generic/software_timer.h
 * Copyright (C) 2016 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef HALM_GENERIC_SOFTWARE_TIMER_H_
#define HALM_GENERIC_SOFTWARE_TIMER_H_
/*----------------------------------------------------------------------------*/
#include <halm/timer.h>
/*----------------------------------------------------------------------------*/
extern const struct EntityClass * const SoftwareTimerFactory;

struct SoftwareTimer;

struct SoftwareTimerFactoryConfig
{
  /** Mandatory: timer for delay measurement. */
  struct Timer *timer;
};

struct SoftwareTimerFactory
{
  struct Entity base;

  struct SoftwareTimer *head;
  struct Timer *timer;

  uint32_t counter;
};
/*----------------------------------------------------------------------------*/
BEGIN_DECLS

void *softwareTimerCreate(void *);

END_DECLS
/*----------------------------------------------------------------------------*/
#endif /* HALM_GENERIC_SOFTWARE_TIMER_H_ */
