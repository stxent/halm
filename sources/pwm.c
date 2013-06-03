/*
 * pwm.c
 * Copyright (C) 2013 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include "pwm.h"
/*----------------------------------------------------------------------------*/
/**
 * Create Pwm object, associated with controller.
 * By default PWM output is stopped.
 * @param controller Pointer to PwmController object.
 * @param pin GPIO pin used as output for pulse width modulated signal.
 * @param value Initial duty cycle value in percents.
 * @return Pointer to new Pwm object on success or zero on error.
 */
void *pwmCreate(void *controller, gpioKey pin, uint8_t value)
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
void pwmSetDutyCycle(void *channel, uint8_t percentage)
{
  ((struct PwmClass *)CLASS(channel))->setDutyCycle(channel, percentage);
}
/*----------------------------------------------------------------------------*/
/**
 * Start or stop pulse width modulation output.
 * @param channel Pointer to Pwm object.
 * @param state PWM channel state, @b true to start or @b false to stop output.
 */
void pwmSetEnabled(void *channel, bool state)
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
void pwmSetPeriod(void *channel, uint16_t period)
{
  ((struct PwmClass *)CLASS(channel))->setPeriod(channel, period);
}
