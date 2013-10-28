/*
 * timer.h
 * Copyright (C) 2013 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef TIMER_H_
#define TIMER_H_
/*----------------------------------------------------------------------------*/
#include <stdbool.h>
#include <stdint.h>
#include <entity.h>
#include <error.h>
/*----------------------------------------------------------------------------*/
/* Class descriptor */
struct TimerClass
{
  CLASS_HEADER

  void (*callback)(void *, void (*)(void *), void *);
  void (*control)(void *, bool);
  void (*setFrequency)(void *, uint32_t);
  void (*setOverflow)(void *, uint32_t);
  uint32_t (*value)(void *);
};
/*----------------------------------------------------------------------------*/
struct Timer
{
  struct Entity parent;
};
/*----------------------------------------------------------------------------*/
/**
 * Set callback function for timer overflow event.
 * @param timer Pointer to Timer object.
 * @param callback Callback function.
 * @param argument Callback function argument.
 */
static inline void timerCallback(void *timer, void (*callback)(void *),
    void *argument)
{
  ((struct TimerClass *)CLASS(timer))->callback(timer, callback, argument);
}
/*----------------------------------------------------------------------------*/
/**
 * Start or stop the timer.
 * @param timer Pointer to Timer object.
 * @param state Timer state: @b true to start timer or @b false to stop timer.
 */
static inline void timerControl(void *timer, bool state)
{
  ((struct TimerClass *)CLASS(timer))->control(timer, state);
}
/*----------------------------------------------------------------------------*/
/**
 * Set fundamental timer frequency.
 * @param timer Pointer to Timer object.
 * @param frequency Frequency in Hz.
 */
static inline void timerSetFrequency(void *timer, uint32_t frequency)
{
  ((struct TimerClass *)CLASS(timer))->setFrequency(timer, frequency);
}
/*----------------------------------------------------------------------------*/
/**
 * Set timer overflow rate.
 * @param timer Pointer to Timer object.
 * @param overflow Number of timer ticks after which overflow event occurs.
 */
static inline void timerSetOverflow(void *timer, uint32_t overflow)
{
  ((struct TimerClass *)CLASS(timer))->setOverflow(timer, overflow);
}
/*----------------------------------------------------------------------------*/
/**
 * Get value of the internal counter.
 * @param timer Pointer to Timer object.
 * @return Value of the counter measured in timer ticks.
 */
static inline uint32_t timerValue(void *timer)
{
  return ((struct TimerClass *)CLASS(timer))->value(timer);
}
/*----------------------------------------------------------------------------*/
#endif /* TIMER_H_ */
