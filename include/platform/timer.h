/*
 * timer.h
 * Copyright (C) 2013 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef TIMER_H_
#define TIMER_H_
/*----------------------------------------------------------------------------*/
#include <stdint.h>
#include <stdbool.h>
#include "entity.h"
#include "error.h"
/*----------------------------------------------------------------------------*/
/* Class descriptor */
struct TimerClass
{
  CLASS_HEADER

  /* Virtual functions with arguments similar to other classes */
  void (*callback)(void *, void (*)(void *), void *);
  void (*control)(void *, bool);
  /* Other virtual functions */
  void (*setFrequency)(void *, uint32_t);
  void (*setOverflow)(void *, uint32_t);
};
/*----------------------------------------------------------------------------*/
struct Timer
{
  struct Entity parent;
};
/*----------------------------------------------------------------------------*/
void timerCallback(void *, void (*)(void *), void *);
void timerControl(void *, bool);
/*----------------------------------------------------------------------------*/
void timerSetFrequency(void *, uint32_t);
void timerSetOverflow(void *, uint32_t);
/*----------------------------------------------------------------------------*/
#endif /* TIMER_H_ */
