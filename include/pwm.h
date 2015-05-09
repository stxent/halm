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
#include <pin.h>
/*----------------------------------------------------------------------------*/
/* Class descriptor */
struct PwmClass
{
  CLASS_HEADER

  uint32_t (*getResolution)(const void *);
  void (*setDuration)(void *, uint32_t);
  void (*setEdges)(void *, uint32_t, uint32_t);
  void (*setEnabled)(void *, bool);
  enum result (*setFrequency)(void *, uint32_t);
};
/*----------------------------------------------------------------------------*/
struct Pwm
{
  struct Entity parent;
};
/*----------------------------------------------------------------------------*/
/**
 * Get channel resolution.
 * @param channel Pointer to a Pwm object.
 * @return Channel resolution in timer ticks.
 */
static inline uint32_t pwmGetResolution(const void *channel)
{
  return ((const struct PwmClass *)CLASS(channel))->getResolution(channel);
}
/*----------------------------------------------------------------------------*/
/**
 * Set duration of the pulse.
 * @param channel Pointer to a Pwm object.
 * @param duration Duration in timer ticks.
 */
static inline void pwmSetDuration(void *channel, uint32_t duration)
{
  ((const struct PwmClass *)CLASS(channel))->setDuration(channel, duration);
}
/*----------------------------------------------------------------------------*/
/**
 * Set times of leading and trailing edges of the pulse.
 * @param channel Pointer to a Pwm object.
 * @param leading Time of a leading edge in timer ticks.
 * @param trailing Time of a trailing edge in timer ticks.
 */
static inline void pwmSetEdges(void *channel, uint32_t leading,
    uint32_t trailing)
{
  ((const struct PwmClass *)CLASS(channel))->setEdges(channel, leading,
      trailing);
}
/*----------------------------------------------------------------------------*/
/**
 * Start or stop pulse width modulation output.
 * @param channel Pointer to a Pwm object.
 * @param state Output state: @b true to start or @b false to stop output.
 */
static inline void pwmSetEnabled(void *channel, bool state)
{
  ((const struct PwmClass *)CLASS(channel))->setEnabled(channel, state);
}
/*----------------------------------------------------------------------------*/
/**
 * Change switching frequency of the channel.
 * @param channel Pointer to a Pwm object.
 * @param frequency New switching frequency in Hz or zero to use default value.
 */
static inline enum result pwmSetFrequency(void *channel, uint32_t frequency)
{
  return ((const struct PwmClass *)CLASS(channel))->setFrequency(channel,
      frequency);
}
/*----------------------------------------------------------------------------*/
#endif /* PWM_H_ */
