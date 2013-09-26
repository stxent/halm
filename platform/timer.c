/*
 * timer.c
 * Copyright (C) 2013 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include "platform/timer.h"
/*----------------------------------------------------------------------------*/
/**
 * Set timer overflow callback function.
 * @param timer Pointer to Timer object.
 * @param callback Callback function.
 * @param argument Callback function argument.
 */
void timerCallback(void *timer, void (*callback)(void *), void *argument)
{
  ((struct TimerClass *)CLASS(timer))->callback(timer, callback, argument);
}
/*----------------------------------------------------------------------------*/
/**
 * Start or stop timer.
 * @param timer Pointer to Timer object.
 * @param state Timer state: @b true to start timer or @b false to stop timer.
 */
void timerControl(void *timer, bool state)
{
  ((struct TimerClass *)CLASS(timer))->control(timer, state);
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
