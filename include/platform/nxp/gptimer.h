/*
 * platform/nxp/gptimer.h
 * Copyright (C) 2013 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef GPTIMER_H_
#define GPTIMER_H_
/*----------------------------------------------------------------------------*/
#include "gptimer_base.h"
/*----------------------------------------------------------------------------*/
extern const struct TimerClass *GpTimer;
/*----------------------------------------------------------------------------*/
/* Symbolic names for two different types of timers on low-performance parts */
enum gpTimerNumber
{
  TIMER_CT16B0 = 0,
  TIMER_CT16B1,
  TIMER_CT32B0,
  TIMER_CT32B1
};
/*----------------------------------------------------------------------------*/
struct GpTimerConfig
{
  uint32_t frequency; /* Mandatory: timer fundamental frequency */
  gpio_t input; /* Optional: clock input pin */
  priority_t priority; /* Optional: timer interrupts priority */
  uint8_t channel; /* Mandatory: timer block */
};
/*----------------------------------------------------------------------------*/
struct GpTimer
{
  struct GpTimerBase parent;

  void (*callback)(void *); /* User interrupt handler */
  void *callbackArgument; /* User interrupt handler argument */
};
/*----------------------------------------------------------------------------*/
#endif /* GPTIMER_H_ */
