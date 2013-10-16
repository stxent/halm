/*
 * platform/nxp/lpc13xx/gptimer_pwm.h
 * Copyright (C) 2013 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef GPTIMER_PWM_H_
#define GPTIMER_PWM_H_
/*----------------------------------------------------------------------------*/
#include <pwm.h>
#include "./gptimer.h"
/*----------------------------------------------------------------------------*/
extern const struct PwmControllerClass *GpTimerPwm;
/*----------------------------------------------------------------------------*/
struct GpTimerPwmConfig
{
  uint32_t frequency; /* Mandatory: cycle frequency */
  uint16_t resolution; /* Mandatory: cycle resolution */
  uint8_t channel; /* Mandatory: timer block */
};
/*----------------------------------------------------------------------------*/
struct GpTimerPwm
{
  struct PwmController parent;

  struct GpTimer *timer;

  uint16_t resolution;
  uint8_t matches; /* Match blocks currently used */
  uint8_t current; /* Match block used for period setup */
};
/*----------------------------------------------------------------------------*/
#endif /* GPTIMER_PWM_H_ */
