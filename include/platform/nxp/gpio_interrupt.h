/*
 * platform/nxp/gpio_interrupt.h
 * Copyright (C) 2014 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef GPIO_INTERRUPT_H_
#define GPIO_INTERRUPT_H_
/*----------------------------------------------------------------------------*/
#include <gpio.h>
#include <interrupt.h>
/*----------------------------------------------------------------------------*/
extern const struct InterruptClass *GpioInterrupt;
/*----------------------------------------------------------------------------*/
enum gpioIntMode
{
  GPIO_RISING,
  GPIO_FALLING,
  GPIO_TOGGLE
};
/*----------------------------------------------------------------------------*/
struct GpioInterruptConfig
{
  gpio_t pin; /* Mandatory: pin used as interrupt source */
  enum gpioIntMode mode; /* External interrupt mode */
};
/*----------------------------------------------------------------------------*/
struct GpioInterrupt
{
  struct Entity parent;

  void (*callback)(void *);
  void *callbackArgument;

  /* Pointer to a next entry in chain */
  struct GpioInterrupt *next;
  /* Descriptor of input pin used as interrupt source */
  union GpioPin pin;
  /* Edge sensitivity mode */
  enum gpioIntMode mode;
};
/*----------------------------------------------------------------------------*/
#endif /* GPIO_INTERRUPT_H_ */
