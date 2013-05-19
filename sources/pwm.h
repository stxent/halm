/*
 * pwm.h
 * Copyright (C) 2013 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef PWM_H_
#define PWM_H_
/*----------------------------------------------------------------------------*/
#include <stdint.h>
#include <stdbool.h>
#include "entity.h"
/*----------------------------------------------------------------------------*/
/* Class descriptor */
struct PwmClass
{
  CLASS_HEADER

  /* Virtual functions */
  void (*setDutyCycle)(void *, uint8_t);
  void (*setEnabled)(void *, bool);
  void (*setPeriod)(void *, uint32_t);
};
/*----------------------------------------------------------------------------*/
struct Pwm
{
  struct Entity parent;
};
/*----------------------------------------------------------------------------*/
void pwmSetDutyCycle(void *, uint8_t);
void pwmSetEnabled(void *, bool);
void pwmSetPeriod(void *, uint32_t);
/*----------------------------------------------------------------------------*/
#endif /* PWM_H_ */
