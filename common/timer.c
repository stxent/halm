/*
 * timer.c
 * Copyright (C) 2013 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include "timer.h"
/*----------------------------------------------------------------------------*/
void timerSetFrequency(void *timer, uint32_t frequency)
{
  return ((struct TimerClass *)CLASS(timer))->setFrequency(timer, frequency);
}
/*----------------------------------------------------------------------------*/
void timerSetHandler(void *timer, void (*handler)(void *), void *parameters)
{
  return ((struct TimerClass *)CLASS(timer))->setHandler(timer, handler,
      parameters);
}
/*----------------------------------------------------------------------------*/
void timerSetOverflow(void *timer, uint32_t overflow)
{
  return ((struct TimerClass *)CLASS(timer))->setOverflow(timer, overflow);
}
/*----------------------------------------------------------------------------*/
void *timerCreateCapture(void *timer, struct Gpio pin, enum captureMode mode)
{
//  if (!timer->Capture)
    return 0; /* Capture mode unsupported */
}
/*----------------------------------------------------------------------------*/
void *timerCreatePwm(void *timer, struct Gpio pin, uint32_t resolution,
    uint32_t fill)
{
//  if (!timer->Pwm)
    return 0; /* PWM mode unsupported */
}
