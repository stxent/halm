/*
 * halm/pwm.h
 * Copyright (C) 2013 xent
 * Project is distributed under the terms of the MIT License
 */

/**
 * @file
 * Abstract class for outputs with Pulse Width Modulation capability.
 */

#ifndef HALM_PWM_H_
#define HALM_PWM_H_
/*----------------------------------------------------------------------------*/
#include <halm/pin.h>
#include <xcore/entity.h>
/*----------------------------------------------------------------------------*/
/* Class descriptor */
struct PwmClass
{
  CLASS_HEADER

  void (*enable)(void *);
  void (*disable)(void *);

  void (*setDuration)(void *, uint32_t);
  void (*setEdges)(void *, uint32_t, uint32_t);
};

struct Pwm
{
  struct Entity base;
};
/*----------------------------------------------------------------------------*/
BEGIN_DECLS

/**
 * Enable pulse width modulation.
 * @param channel Pointer to a Pwm object.
 */
static inline void pwmEnable(void *channel)
{
  ((const struct PwmClass *)CLASS(channel))->enable(channel);
}

/**
 * Disable pulse width modulation.
 * @param channel Pointer to a Pwm object.
 */
static inline void pwmDisable(void *channel)
{
  ((const struct PwmClass *)CLASS(channel))->disable(channel);
}

/**
 * Set duration of the pulse.
 * @param channel Pointer to a Pwm object.
 * @param duration Duration in timer ticks.
 */
static inline void pwmSetDuration(void *channel, uint32_t duration)
{
  ((const struct PwmClass *)CLASS(channel))->setDuration(channel, duration);
}

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

END_DECLS
/*----------------------------------------------------------------------------*/
#endif /* HALM_PWM_H_ */
