/*
 * base_timer.h
 * Copyright (C) 2013 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef BASE_TIMER_PWM_H_
#define BASE_TIMER_PWM_H_
/*----------------------------------------------------------------------------*/
#include "platform/pwm.h"
#include "./base_timer.h"
/*----------------------------------------------------------------------------*/
extern const struct PwmControllerClass *BaseTimerPwm;
/*----------------------------------------------------------------------------*/
struct BaseTimerPwmConfig
{
  uint32_t frequency; /* Mandatory: cycle frequency */
  uint16_t resolution; /* Mandatory: cycle resolution */
  uint8_t channel; /* Mandatory: timer block */
};
/*----------------------------------------------------------------------------*/
struct BaseTimerPwm
{
  struct PwmController parent;

  struct BaseTimer *timer;

  uint16_t resolution;
  uint8_t matches; /* Match blocks currently used */
  uint8_t current; /* Match block used for period setup */
};
/*----------------------------------------------------------------------------*/
#endif /* BASE_TIMER_PWM_H_ */
