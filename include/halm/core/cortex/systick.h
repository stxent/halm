/*
 * halm/core/cortex/systick.h
 * Copyright (C) 2013 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_CORE_CORTEX_SYSTICK_H_
#define HALM_CORE_CORTEX_SYSTICK_H_
/*----------------------------------------------------------------------------*/
#include <halm/irq.h>
#include <halm/timer.h>
/*----------------------------------------------------------------------------*/
extern const struct TimerClass * const SysTick;

/*
 * All configuration fields are optional, it is allowed to pass
 * a null pointer as the constructor argument.
 */
struct SysTickConfig
{
  /** Optional: interrupt priority. */
  IrqPriority priority;
};

struct SysTick
{
  struct Timer base;

  /* User interrupt handler */
  void (*callback)(void *);
  /* Argument passed to the user interrupt handler */
  void *callbackArgument;
};
/*----------------------------------------------------------------------------*/
#endif /* HALM_CORE_CORTEX_SYSTICK_H_ */
