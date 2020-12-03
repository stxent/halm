/*
 * halm/platform/lpc/rit.h
 * Copyright (C) 2018 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef HALM_PLATFORM_LPC_RIT_H_
#define HALM_PLATFORM_LPC_RIT_H_
/*----------------------------------------------------------------------------*/
#include <halm/irq.h>
#include <halm/timer.h>
/*----------------------------------------------------------------------------*/
extern const struct TimerClass * const Rit;

/*
 * All configuration fields are optional, it is allowed to pass
 * a null pointer as the constructor argument.
 */
struct RitConfig
{
  /** Optional: interrupt priority. */
  IrqPriority priority;
};

struct Rit
{
  struct Timer parent;

  /* User interrupt handler */
  void (*callback)(void *);
  /* User interrupt handler argument */
  void *callbackArgument;
};
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_LPC_RIT_H_ */
