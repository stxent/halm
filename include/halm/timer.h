/*
 * halm/timer.h
 * Copyright (C) 2013 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

/**
 * @file
 * Abstract timer class.
 */

#ifndef HALM_TIMER_H_
#define HALM_TIMER_H_
/*----------------------------------------------------------------------------*/
#include <stdbool.h>
#include <stdint.h>
#include <xcore/entity.h>
/*----------------------------------------------------------------------------*/
/* Class descriptor */
struct TimerClass
{
  CLASS_HEADER

  void (*callback)(void *, void (*)(void *), void *);
  void (*setEnabled)(void *, bool);
  enum result (*setFrequency)(void *, uint32_t);
  enum result (*setOverflow)(void *, uint32_t);
  enum result (*setValue)(void *, uint32_t);
  uint32_t (*value)(const void *);
};
/*----------------------------------------------------------------------------*/
struct Timer
{
  struct Entity base;
};
/*----------------------------------------------------------------------------*/
/**
 * Set callback function for timer overflow event.
 * @param timer Pointer to a Timer object.
 * @param callback Callback function.
 * @param argument Callback function argument.
 */
static inline void timerCallback(void *timer, void (*callback)(void *),
    void *argument)
{
  ((const struct TimerClass *)CLASS(timer))->callback(timer, callback,
      argument);
}
/*----------------------------------------------------------------------------*/
/**
 * Start or stop the timer.
 * @param timer Pointer to a Timer object.
 * @param state Timer state: @b true to start timer or @b false to stop timer.
 */
static inline void timerSetEnabled(void *timer, bool state)
{
  ((const struct TimerClass *)CLASS(timer))->setEnabled(timer, state);
}
/*----------------------------------------------------------------------------*/
/**
 * Set fundamental timer frequency.
 * @param timer Pointer to a Timer object.
 * @param frequency Frequency in Hz.
 * @return @b E_OK on success.
 */
static inline enum result timerSetFrequency(void *timer, uint32_t frequency)
{
  return ((const struct TimerClass *)CLASS(timer))->setFrequency(timer,
      frequency);
}
/*----------------------------------------------------------------------------*/
/**
 * Set timer overflow rate.
 * @param timer Pointer to a Timer object.
 * @param overflow Number of timer ticks after which overflow event occurs.
 * @return @b E_OK on success.
 */
static inline enum result timerSetOverflow(void *timer, uint32_t overflow)
{
  return ((const struct TimerClass *)CLASS(timer))->setOverflow(timer,
      overflow);
}
/*----------------------------------------------------------------------------*/
/**
 * Set timer value.
 * @param timer Pointer to a Timer object.
 * @param value New timer value.
 * @return @b E_OK on success.
 */
static inline enum result timerSetValue(void *timer, uint32_t value)
{
  return ((const struct TimerClass *)CLASS(timer))->setValue(timer, value);
}
/*----------------------------------------------------------------------------*/
/**
 * Get value of the internal counter.
 * @param timer Pointer to a Timer object.
 * @return Value of the counter measured in timer ticks.
 */
static inline uint32_t timerValue(const void *timer)
{
  return ((const struct TimerClass *)CLASS(timer))->value(timer);
}
/*----------------------------------------------------------------------------*/
#endif /* HALM_TIMER_H_ */
