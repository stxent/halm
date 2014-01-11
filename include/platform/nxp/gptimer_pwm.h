/*
 * platform/nxp/gptimer_pwm.h
 * Copyright (C) 2013 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef GPTIMER_PWM_H_
#define GPTIMER_PWM_H_
/*----------------------------------------------------------------------------*/
#include <pwm.h>
#include "gptimer_base.h"
/*----------------------------------------------------------------------------*/
extern const struct EntityClass *GpTimerPwmUnit;
extern const struct PwmClass *GpTimerPwm;
/*----------------------------------------------------------------------------*/
struct GpTimerPwmUnitConfig
{
  uint32_t frequency; /* Mandatory: cycle frequency */
  uint32_t resolution; /* Mandatory: cycle resolution */
  uint8_t channel; /* Mandatory: timer block */
};
/*----------------------------------------------------------------------------*/
struct GpTimerPwmUnit
{
  struct GpTimerBase parent;

  uint32_t resolution;
  uint8_t current; /* Match block used for period setup */
  uint8_t matches; /* Match blocks currently in use */
};
/*----------------------------------------------------------------------------*/
struct GpTimerPwmConfig
{
  struct GpTimerPwmUnit *parent; /* Mandatory: peripheral unit */
  uint32_t duration; /* Optional: initial duration in timer ticks */
  gpio_t pin; /* Mandatory: pin used as output for modulated signal */
};
/*----------------------------------------------------------------------------*/
struct GpTimerPwm
{
  struct Pwm parent;

  /* Pointer to a parent unit */
  struct GpTimerPwmUnit *unit;
  /* Pointer to a match register */
  uint32_t *value;
  /* Match channel number */
  uint8_t channel;
};
/*----------------------------------------------------------------------------*/
void *gpTimerPwmCreate(void *, gpio_t, uint32_t);
/*----------------------------------------------------------------------------*/
#endif /* GPTIMER_PWM_H_ */
