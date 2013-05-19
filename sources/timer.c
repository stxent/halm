/*
 * timer.c
 * Copyright (C) 2013 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include "timer.h"
/*----------------------------------------------------------------------------*/
/**
 * Set timer overflow callback.
 * @param timer Pointer to Timer object.
 * @param callback Callback function.
 * @param parameters Callback function parameters.
 */
void timerSetCallback(void *timer, void (*callback)(void *), void *parameters)
{
  ((struct TimerClass *)CLASS(timer))->setCallback(timer, callback, parameters);
}
/*----------------------------------------------------------------------------*/
/**
 * Start or stop timer.
 * @param timer Pointer to Timer object.
 * @param state Timer state: @b true to start timer or @b false to stop timer.
 */
void timerSetEnabled(void *timer, bool state)
{
  ((struct TimerClass *)CLASS(timer))->setEnabled(timer, state);
}
/*----------------------------------------------------------------------------*/
/**
 * Set fundamental timer frequency.
 * @param timer Pointer to Timer object.
 * @param frequency Frequency in Hz.
 */
void timerSetFrequency(void *timer, uint32_t frequency)
{
  ((struct TimerClass *)CLASS(timer))->setFrequency(timer, frequency);
}
/*----------------------------------------------------------------------------*/
/**
 * Set timer overflow rate.
 * @param timer Pointer to Timer object.
 * @param overflow Number of timer ticks after which overflow event occurs.
 */
void timerSetOverflow(void *timer, uint32_t overflow)
{
  ((struct TimerClass *)CLASS(timer))->setOverflow(timer, overflow);
}
/*----------------------------------------------------------------------------*/
/**
 * Create event capture object, associated with timer.
 * By default event capture is stopped.
 * @param timer Pointer to Timer object.
 * @param pin GPIO pin used as source input.
 * @param mode Capture mode.
 * @return Pointer to new Capture object on success or zero on error.
 */
void *timerCreateCapture(void *timer, struct Gpio pin, enum captureMode mode)
{
  return ((struct TimerClass *)CLASS(timer))->createCapture(timer, pin, mode);
}
/*----------------------------------------------------------------------------*/
/**
 * Create Pwm object, associated with timer.
 * By default PWM output is stopped.
 * @param timer Pointer to Timer object.
 * @param pin GPIO pin used as output for pulse width modulated signal.
 * @param resolution PWM resolution.
 * @return Pointer to new Pwm object on success or zero on error.
 */
void *timerCreatePwm(void *timer, struct Gpio pin, uint32_t resolution)
{
  return ((struct TimerClass *)CLASS(timer))->createPwm(timer, pin, resolution);
}
