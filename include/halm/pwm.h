/*
 * halm/pwm.h
 * Copyright (C) 2013 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

/**
 * @file
 * Abstract class for outputs with Pulse Width Modulation capability.
 */

#ifndef HALM_PWM_H_
#define HALM_PWM_H_
/*----------------------------------------------------------------------------*/
#include <stdbool.h>
#include <xcore/entity.h>
#include <halm/pin.h>
/*----------------------------------------------------------------------------*/
/* Class descriptor */
struct PwmClass
{
  CLASS_HEADER

  void (*enable)(void *);
  void (*disable)(void *);

  uint32_t (*getResolution)(const void *);
  void (*setDuration)(void *, uint32_t);
  void (*setEdges)(void *, uint32_t, uint32_t);
  enum Result (*setFrequency)(void *, uint32_t);
};
/*----------------------------------------------------------------------------*/
struct Pwm
{
  struct Entity base;
};
/*----------------------------------------------------------------------------*/
/**
 * Enable pulse width modulation.
 * @param channel Pointer to a Pwm object.
 */
static inline void pwmEnable(void *channel)
{
  ((const struct PwmClass *)CLASS(channel))->enable(channel);
}
/*----------------------------------------------------------------------------*/
/**
 * Disable pulse width modulation.
 * @param channel Pointer to a Pwm object.
 */
static inline void pwmDisable(void *channel)
{
  ((const struct PwmClass *)CLASS(channel))->disable(channel);
}
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
 * @param leading Time of the leading edge in timer ticks.
 * @param trailing Time of the trailing edge in timer ticks.
 */
static inline void pwmSetEdges(void *channel, uint32_t leading,
    uint32_t trailing)
{
  ((const struct PwmClass *)CLASS(channel))->setEdges(channel, leading,
      trailing);
}
/*----------------------------------------------------------------------------*/
/**
 * Change switching frequency of the channel.
 * @param channel Pointer to a Pwm object.
 * @param frequency New switching frequency in Hz or zero to use default value.
 */
static inline enum Result pwmSetFrequency(void *channel, uint32_t frequency)
{
  return ((const struct PwmClass *)CLASS(channel))->setFrequency(channel,
      frequency);
}
/*----------------------------------------------------------------------------*/
#endif /* HALM_PWM_H_ */
