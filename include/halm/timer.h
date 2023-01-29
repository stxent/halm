/*
 * halm/timer.h
 * Copyright (C) 2013 xent
 * Project is distributed under the terms of the MIT License
 */

/**
 * @file
 * Abstract Timer class.
 */

#ifndef HALM_TIMER_H_
#define HALM_TIMER_H_
/*----------------------------------------------------------------------------*/
#include <xcore/entity.h>
#include <stdbool.h>
#include <stdint.h>
/*----------------------------------------------------------------------------*/
/* Class descriptor */
struct TimerClass
{
  CLASS_HEADER

  void (*enable)(void *);
  void (*disable)(void *);

  void (*setAutostop)(void *, bool);
  void (*setCallback)(void *, void (*)(void *), void *);

  uint32_t (*getFrequency)(const void *);
  void (*setFrequency)(void *, uint32_t);

  uint32_t (*getOverflow)(const void *);
  void (*setOverflow)(void *, uint32_t);

  uint32_t (*getValue)(const void *);
  void (*setValue)(void *, uint32_t);
};

struct Timer
{
  struct Entity base;
};
/*----------------------------------------------------------------------------*/
BEGIN_DECLS

/**
 * Enable the timer and continue counting from the previous value.
 * @param timer Pointer to a Timer object.
 */
static inline void timerEnable(void *timer)
{
  ((const struct TimerClass *)CLASS(timer))->enable(timer);
}

/**
 * Stop the timer and preserve current value.
 * @param timer Pointer to a Timer object.
 */
static inline void timerDisable(void *timer)
{
  ((const struct TimerClass *)CLASS(timer))->disable(timer);
}

/**
 * Enable or disable the autostop function of a timer.
 * @param timer Pointer to a Timer object.
 * @param state Enable or disable the autostop function.
 */
static inline void timerSetAutostop(void *timer, bool state)
{
  ((const struct TimerClass *)CLASS(timer))->setAutostop(timer, state);
}

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

/**
 * Get the frequency of the timer.
 * @param timer Pointer to a Timer object.
 * @return Frequency of the timer in Hz.
 */
static inline uint32_t timerGetFrequency(const void *timer)
{
  return ((const struct TimerClass *)CLASS(timer))->getFrequency(timer);
}

/**
 * Set the frequency of the timer.
 * @param timer Pointer to a Timer object.
 * @param frequency Frequency in Hz.
 */
static inline void timerSetFrequency(void *timer, uint32_t frequency)
{
  ((const struct TimerClass *)CLASS(timer))->setFrequency(timer, frequency);
}

/**
 * Get the timer overflow rate.
 * @param timer Pointer to a Timer object.
 * @return Number of timer ticks after which overflow event occurs.
 */
static inline uint32_t timerGetOverflow(const void *timer)
{
  return ((const struct TimerClass *)CLASS(timer))->getOverflow(timer);
}

/**
 * Set the timer overflow rate.
 * @param timer Pointer to a Timer object.
 * @param overflow Number of timer ticks after which overflow event occurs.
 */
static inline void timerSetOverflow(void *timer, uint32_t overflow)
{
  ((const struct TimerClass *)CLASS(timer))->setOverflow(timer, overflow);
}

/**
 * Get the current value of the timer.
 * @param timer Pointer to a Timer object.
 * @return Value of the counter measured in timer ticks.
 */
static inline uint32_t timerGetValue(const void *timer)
{
  return ((const struct TimerClass *)CLASS(timer))->getValue(timer);
}

/**
 * Set the current value.
 * @param timer Pointer to a Timer object.
 * @param value New timer value.
 */
static inline void timerSetValue(void *timer, uint32_t value)
{
  ((const struct TimerClass *)CLASS(timer))->setValue(timer, value);
}

END_DECLS
/*----------------------------------------------------------------------------*/
/* Class descriptor */
struct Timer64Class
{
  struct TimerClass base;

  uint64_t (*getValue64)(const void *);
  void (*setValue64)(void *, uint64_t);
};

struct Timer64
{
  struct Timer base;
};
/*----------------------------------------------------------------------------*/
BEGIN_DECLS

/**
 * Get the current value of the timer.
 * @param timer Pointer to a Timer64 object.
 * @return Value of the counter measured in timer ticks.
 */
static inline uint64_t timerGetValue64(const void *timer)
{
  return ((const struct Timer64Class *)CLASS(timer))->getValue64(timer);
}

/**
 * Set the current value.
 * @param timer Pointer to a Timer64 object.
 * @param value New timer value.
 */
static inline void timerSetValue64(void *timer, uint64_t value)
{
  ((const struct Timer64Class *)CLASS(timer))->setValue64(timer, value);
}

END_DECLS
/*----------------------------------------------------------------------------*/
#endif /* HALM_TIMER_H_ */
