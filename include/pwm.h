/*
 * pwm.h
 * Copyright (C) 2013 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef PWM_H_
#define PWM_H_
/*----------------------------------------------------------------------------*/
#include <stdbool.h>
#include <entity.h>
#include <gpio.h>
/*----------------------------------------------------------------------------*/
/* Class descriptor */
struct PwmControllerClass
{
  CLASS_HEADER

  /* Virtual functions */
  void *(*create)(void *, gpio_t, uint8_t);
};
/*----------------------------------------------------------------------------*/
/* Class descriptor */
struct PwmClass
{
  CLASS_HEADER

  /* Virtual functions */
  void (*setDutyCycle)(void *, uint8_t);
  void (*setEnabled)(void *, bool);
  void (*setPeriod)(void *, uint16_t);
};
/*----------------------------------------------------------------------------*/
struct PwmController
{
  struct Entity parent;
};
/*----------------------------------------------------------------------------*/
struct Pwm
{
  struct Entity parent;
};
/*----------------------------------------------------------------------------*/
/**
 * Create Pwm object, associated with controller.
 * By default PWM output is stopped.
 * @param controller Pointer to PwmController object.
 * @param pin GPIO pin used as output for pulse width modulated signal.
 * @param value Initial duty cycle value in percents.
 * @return Pointer to new Pwm object on success or zero on error.
 */
static inline void *pwmCreate(void *controller, gpio_t pin, uint8_t value)
{
  return ((struct PwmControllerClass *)CLASS(controller))->create(controller,
      pin, value);
}
/*----------------------------------------------------------------------------*/
/**
 * Set duty cycle of pulse width modulated signal.
 * @param channel Pointer to Pwm object.
 * @param percentage Duty cycle in percents.
 */
static inline void pwmSetDutyCycle(void *channel, uint8_t percentage)
{
  ((struct PwmClass *)CLASS(channel))->setDutyCycle(channel, percentage);
}
/*----------------------------------------------------------------------------*/
/**
 * Start or stop pulse width modulation output.
 * @param channel Pointer to Pwm object.
 * @param state PWM channel state, @b true to start or @b false to stop output.
 */
static inline void pwmSetEnabled(void *channel, bool state)
{
  ((struct PwmClass *)CLASS(channel))->setEnabled(channel, state);
}
/*----------------------------------------------------------------------------*/
/**
 * Set length of period when PWM output is enabled.
 * More precise version of pwmSetPeriod function.
 * @param channel Pointer to Pwm object.
 * @param period Period in timer ticks.
 */
static inline void pwmSetPeriod(void *channel, uint16_t period)
{
  ((struct PwmClass *)CLASS(channel))->setPeriod(channel, period);
}
/*----------------------------------------------------------------------------*/
#endif /* PWM_H_ */
