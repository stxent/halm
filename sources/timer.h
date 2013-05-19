/*
 * timer.h
 * Copyright (C) 2013 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef TIMER_H_
#define TIMER_H_
/*----------------------------------------------------------------------------*/
#include <stdint.h>
#include "entity.h"
#include "error.h"
#include "gpio.h"
/*----------------------------------------------------------------------------*/
enum timerMode
{
  TIMER_NORMAL = 0,
  TIMER_CAPTURE,
  TIMER_MATCH, //FIXME Remove?
  TIMER_PWM
};
/*----------------------------------------------------------------------------*/
enum captureMode
{
  CAPTURE_RISING,
  CAPTURE_FALLIND,
  CAPTURE_TOGGLE
};
/*----------------------------------------------------------------------------*/
/* Class descriptor */
struct TimerClass
{
  CLASS_HEADER

  /* Virtual functions */
  void (*setFrequency)(void *, uint32_t);
  void (*setCallback)(void *, void (*)(void *), void *);
  void (*setOverflow)(void *, uint32_t);
  void *(*createCapture)(void *, struct Gpio, enum captureMode);
  void *(*createPwm)(void *, struct Gpio, uint32_t, uint32_t);
};
/*----------------------------------------------------------------------------*/
struct Timer
{
  struct Entity parent;
};
/*----------------------------------------------------------------------------*/
void timerSetFrequency(void *, uint32_t);
void timerSetCallback(void *, void (*)(void *), void *);
void timerSetOverflow(void *, uint32_t);
void *timerCreateCapture(void *, struct Gpio, enum captureMode);
void *timerCreatePwm(void *, struct Gpio, uint32_t, uint32_t);
/*----------------------------------------------------------------------------*/
#endif /* TIMER_H_ */
