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

  void (*setCallback)(void *, void (*)(void *), void *);
  void (*setEnabled)(void *, bool);

  uint32_t (*getFrequency)(const void *);
  void (*setFrequency)(void *, uint32_t);

  uint32_t (*getOverflow)(const void *);
  void (*setOverflow)(void *, uint32_t);

  uint32_t (*getValue)(const void *);
  void (*setValue)(void *, uint32_t);
};
/*----------------------------------------------------------------------------*/
struct Timer
{
  struct Entity base;
};
/*----------------------------------------------------------------------------*/
/**
 * Set the callback function for the timer overflow event.
 * @param timer Pointer to a Timer object.
 * @param callback Callback function.
 * @param argument Callback function argument.
 */
static inline void timerSetCallback(void *timer, void (*callback)(void *),
    void *argument)
{
  ((const struct TimerClass *)CLASS(timer))->setCallback(timer, callback,
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
 * Get the current frequency of the timer.
 * @param timer Pointer to a Timer object.
 * @return Frequency of the timer in Hz.
 */
static inline uint32_t timerGetFrequency(const void *timer)
{
  return ((const struct TimerClass *)CLASS(timer))->getFrequency(timer);
}
/*----------------------------------------------------------------------------*/
/**
 * Set the frequency of the timer.
 * @param timer Pointer to a Timer object.
 * @param frequency Frequency in Hz.
 */
static inline void timerSetFrequency(void *timer, uint32_t frequency)
{
  ((const struct TimerClass *)CLASS(timer))->setFrequency(timer, frequency);
}
/*----------------------------------------------------------------------------*/
/**
 * Get the timer overflow rate.
 * @param timer Pointer to a Timer object.
 * @return Number of timer ticks after which overflow event occurs.
 */
static inline uint32_t timerGetOverflow(const void *timer)
{
  return ((const struct TimerClass *)CLASS(timer))->getOverflow(timer);
}
/*----------------------------------------------------------------------------*/
/**
 * Set the timer overflow rate.
 * @param timer Pointer to a Timer object.
 * @param overflow Number of timer ticks after which overflow event occurs.
 */
static inline void timerSetOverflow(void *timer, uint32_t overflow)
{
  ((const struct TimerClass *)CLASS(timer))->setOverflow(timer, overflow);
}
/*----------------------------------------------------------------------------*/
/**
 * Get the current value of a timer.
 * @param timer Pointer to a Timer object.
 * @return Value of the counter measured in timer ticks.
 */
static inline uint32_t timerGetValue(const void *timer)
{
  return ((const struct TimerClass *)CLASS(timer))->getValue(timer);
}
/*----------------------------------------------------------------------------*/
/**
 * Set the timer value.
 * @param timer Pointer to a Timer object.
 * @param value New timer value.
 */
static inline void timerSetValue(void *timer, uint32_t value)
{
  ((const struct TimerClass *)CLASS(timer))->setValue(timer, value);
}
/*----------------------------------------------------------------------------*/
#endif /* HALM_TIMER_H_ */
