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

struct SysTickTimerConfig
{
  /** Mandatory: timer fundamental frequency. */
  uint32_t frequency;
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

  /* Desired timer frequency */
  uint32_t frequency;
  /* Overflow value in ticks of desired frequency */
  uint32_t overflow;
};
/*----------------------------------------------------------------------------*/
#endif /* HALM_CORE_CORTEX_SYSTICK_H_ */
