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

  uint32_t (*getFrequency)(const void *);
  enum result (*setFrequency)(void *, uint32_t);

  enum result (*setOverflow)(void *, uint32_t);

  uint32_t (*getValue)(const void *);
  enum result (*setValue)(void *, uint32_t);
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
 * @return @b E_OK on success.
 */
static inline enum result timerSetFrequency(void *timer, uint32_t frequency)
{
  return ((const struct TimerClass *)CLASS(timer))->setFrequency(timer,
      frequency);
}
/*----------------------------------------------------------------------------*/
/**
 * Set the timer overflow rate.
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
 * @return @b E_OK on success.
 */
static inline enum result timerSetValue(void *timer, uint32_t value)
{
  return ((const struct TimerClass *)CLASS(timer))->setValue(timer, value);
}
/*----------------------------------------------------------------------------*/
#endif /* HALM_TIMER_H_ */
