/*
 * core/cortex/systick.h
 * Copyright (C) 2013 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef CORE_CORTEX_SYSTICK_H_
#define CORE_CORTEX_SYSTICK_H_
/*----------------------------------------------------------------------------*/
#include <irq.h>
#include <timer.h>
/*----------------------------------------------------------------------------*/
extern const struct TimerClass * const SysTickTimer;
/*----------------------------------------------------------------------------*/
struct SysTickTimerConfig
{
  /** Mandatory: timer fundamental frequency. */
  uint32_t frequency;
  /** Optional: interrupt priority. */
  priority_t priority;
  /** Optional: do not enable the timer after the initialization. */
  bool disabled;
};
/*----------------------------------------------------------------------------*/
struct SysTickTimer
{
  struct Timer parent;

  /* Hardware interrupt handler */
  void (*handler)(void *);
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
#endif /* CORE_CORTEX_SYSTICK_H_ */
