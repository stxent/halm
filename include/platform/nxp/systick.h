/*
 * systick.h
 * Copyright (C) 2013 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef SYSTICK_H_
#define SYSTICK_H_
/*----------------------------------------------------------------------------*/
#include "platform/timer.h"
/*----------------------------------------------------------------------------*/
extern const struct TimerClass *SysTickTimer;
/*----------------------------------------------------------------------------*/
struct SysTickTimerConfig
{
  uint32_t frequency; /* Mandatory: timer fundamental frequency */
};
/*----------------------------------------------------------------------------*/
struct SysTickTimer
{
  struct Timer parent;

  void (*handler)(void *); /* Hardware interrupt handler */
  void (*callback)(void *); /* User interrupt handler */
  void *callbackArgument; /* User interrupt handler argument */

  uint32_t frequency, overflow;
};
/*----------------------------------------------------------------------------*/
#endif /* SYSTICK_H_ */
