/*
 * halm/core/cortex/systick.h
 * Copyright (C) 2013 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef HALM_CORE_CORTEX_SYSTICK_H_
#define HALM_CORE_CORTEX_SYSTICK_H_
/*----------------------------------------------------------------------------*/
#include <halm/irq.h>
#include <halm/timer.h>
/*----------------------------------------------------------------------------*/
extern const struct TimerClass * const SysTickTimer;

/*
 * All configuration fields are optional, it is allowed to pass
 * a null pointer as the constructor argument.
 */
struct SysTickTimerConfig
{
  /** Optional: interrupt priority. */
  IrqPriority priority;
};

struct SysTickTimer
{
  struct Timer parent;

  /* User interrupt handler */
  void (*callback)(void *);
  /* User interrupt handler argument */
  void *callbackArgument;
};
/*----------------------------------------------------------------------------*/
#endif /* HALM_CORE_CORTEX_SYSTICK_H_ */
