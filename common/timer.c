/*
 * timer.c
 * Copyright (C) 2013 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include "timer.h"
/*----------------------------------------------------------------------------*/
/**
 * Set fundamental timer frequency.
 * @param timer Pointer to Timer object.
 * @param frequency New frequency in Hz.
 */
void timerSetFrequency(void *timer, uint32_t frequency)
{
  return ((struct TimerClass *)CLASS(timer))->setFrequency(timer, frequency);
}
/*----------------------------------------------------------------------------*/
/* Set overflow handler, arguments: pointer to handler function */
void timerSetCallback(void *timer, void (*callback)(void *), void *parameters)
{
  return ((struct TimerClass *)CLASS(timer))->setCallback(timer, callback,
      parameters);
}
/*----------------------------------------------------------------------------*/
/* Set timer overflow rate, arguments: number of timer clocks */
void timerSetOverflow(void *timer, uint32_t overflow)
{
  return ((struct TimerClass *)CLASS(timer))->setOverflow(timer, overflow);
}
/*----------------------------------------------------------------------------*/
/*
 * Use the pin for event capturing, arguments: pointer to Capture object,
 * input pin, sensitive edge.
 */
void *timerCreateCapture(void *timer, struct Gpio pin, enum captureMode mode)
{
//  if (!timer->Capture)
    return 0; /* Capture mode unsupported */
}
/*----------------------------------------------------------------------------*/
/*
 * Output pulse width modulated signal to specified pin, arguments:
 * pointer to Pwm object, output pin, resolution, initial fill
 */
void *timerCreatePwm(void *timer, struct Gpio pin, uint32_t resolution,
    uint32_t fill)
{
//  if (!timer->Pwm)
    return 0; /* PWM mode unsupported */
}
