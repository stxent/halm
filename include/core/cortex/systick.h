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
  uint32_t frequency; /* Mandatory: timer fundamental frequency */
  priority_t priority; /* Optional: interrupt priority */
};
/*----------------------------------------------------------------------------*/
struct SysTickTimer
{
  struct Timer parent;

  void (*handler)(void *); /* Hardware interrupt handler */
  void (*callback)(void *); /* User interrupt handler */
  void *callbackArgument; /* User interrupt handler argument */

  uint32_t frequency;
  uint32_t overflow;
};
/*----------------------------------------------------------------------------*/
#endif /* CORE_CORTEX_SYSTICK_H_ */
