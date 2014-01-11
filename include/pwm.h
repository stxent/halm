/*
 * pwm.h
 * Copyright (C) 2013 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

/**
 * @file
 * Abstract class for outputs with Pulse Width Modulation capability.
 */

#ifndef PWM_H_
#define PWM_H_
/*----------------------------------------------------------------------------*/
#include <stdbool.h>
#include <entity.h>
#include <gpio.h>
/*----------------------------------------------------------------------------*/
/* Class descriptor */
struct PwmClass
{
  CLASS_HEADER

  uint32_t (*getResolution)(void *);
  void (*setDuration)(void *, uint32_t);
  void (*setEdges)(void *, uint32_t, uint32_t);
  void (*setEnabled)(void *, bool);
};
/*----------------------------------------------------------------------------*/
struct Pwm
{
  struct Entity parent;
};
/*----------------------------------------------------------------------------*/
/**
 * Get channel resolution.
 * @param channel Pointer to Pwm object.
 * @return Channel resolution in timer ticks.
 */
static inline uint32_t pwmGetResolution(void *channel)
{
  return ((struct PwmClass *)CLASS(channel))->getResolution(channel);
}
/*----------------------------------------------------------------------------*/
/**
 * Set duration of the pulse when output is in active state.
 * @param channel Pointer to Pwm object.
 * @param duration Duration in timer ticks.
 */
static inline void pwmSetDuration(void *channel, uint32_t duration)
{
  ((struct PwmClass *)CLASS(channel))->setDuration(channel, duration);
}
/*----------------------------------------------------------------------------*/
/**
 * Set times of leading and trailing edges of the pulse.
 * @param channel Pointer to Pwm object.
 * @param leading Time of a leading edge in timer ticks.
 * @param trailing Time of a trailing edge in timer ticks.
 */
static inline void pwmSetEdges(void *channel, uint32_t leading,
    uint32_t trailing)
{
  ((struct PwmClass *)CLASS(channel))->setEdges(channel, leading, trailing);
}
/*----------------------------------------------------------------------------*/
/**
 * Start or stop pulse width modulation output.
 * @param channel Pointer to Pwm object.
 * @param state Output state: @b true to start or @b false to stop output.
 */
static inline void pwmSetEnabled(void *channel, bool state)
{
  ((struct PwmClass *)CLASS(channel))->setEnabled(channel, state);
}
/*----------------------------------------------------------------------------*/
#endif /* PWM_H_ */
