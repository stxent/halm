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
struct GpTimerConfig
{
  uint32_t frequency; /* Mandatory: timer fundamental frequency */
  gpio_t input; /* Optional: clock input pin */
  priority_t priority; /* Optional: timer interrupt priority */
  uint8_t channel; /* Mandatory: timer block */
  uint8_t event; /* Optional: match channel used as reset event source */
};
/*----------------------------------------------------------------------------*/
struct GpTimer
{
  struct GpTimerBase parent;

  void (*callback)(void *);
  void *callbackArgument;

  /* Match channel used for counter reset */
  uint8_t event;
};
/*----------------------------------------------------------------------------*/
#endif /* GPTIMER_H_ */
