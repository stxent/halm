/*
 * halm/platform/lpc/lpc43xx/atimer.h
 * Copyright (C) 2023 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_LPC_LPC43XX_ATIMER_H_
#define HALM_PLATFORM_LPC_LPC43XX_ATIMER_H_
/*----------------------------------------------------------------------------*/
#include <halm/irq.h>
#include <halm/timer.h>
/*----------------------------------------------------------------------------*/
extern const struct TimerClass * const Atimer;

struct Atimer
{
  struct Timer base;

  void (*callback)(void *);
  void *callbackArgument;
};
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_LPC_LPC43XX_ATIMER_H_ */
