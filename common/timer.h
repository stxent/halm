/*
 * timer.h
 * Copyright (C) 2013 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef TIMER_H_
#define TIMER_H_
/*----------------------------------------------------------------------------*/
#include <stdint.h>
/*----------------------------------------------------------------------------*/
#include "error.h"
#include "entity.h"
#include "gpio.h"
/*----------------------------------------------------------------------------*/
enum timerMode {
  TIMER_NORMAL = 0,
  TIMER_CAPTURE,
  TIMER_MATCH, //FIXME Remove?
  TIMER_PWM
};
/*----------------------------------------------------------------------------*/
enum captureMode {
  CAPTURE_RISING,
  CAPTURE_FALLIND,
  CAPTURE_TOGGLE
};
/*----------------------------------------------------------------------------*/
/* Class descriptor */
struct TimerClass
{
  CLASS_GENERATOR

  /* Pointers to subclasses */
  const void *Capture;
  const void *Pwm;

  /* Virtual functions */
  /* Set fundamental timer frequency, arguments: new frequency in Hz */
  void (*setFrequency)(void *, uint32_t);
  /* Set overflow handler, arguments: pointer to handler function */
  void (*setHandler)(void *, void (*)(void *), void *);
  /* Set timer overflow rate, arguments: number of timer clocks */
  void (*setOverflow)(void *, uint32_t);
  /*
   * Use the pin for event capturing, arguments: pointer to Capture object,
   * input pin, sensitive edge.
   */
  enum result (*createCapture)(void *, void *, struct Gpio, enum captureMode);
  /*
   * Output pulse width modulated signal to specified pin, arguments:
   * pointer to Pwm object, output pin, resolution, initial fill
   */
  enum result (*createPwm)(void *, void *, struct Gpio, uint32_t, uint32_t);

  /* Interrupt handler, arguments: Timer descriptor assigned to peripheral */
  void (*handler)(void *);
};
/*----------------------------------------------------------------------------*/
struct Timer
{
  struct Entity parent;
};
/*----------------------------------------------------------------------------*/
void timerSetFrequency(void *, uint32_t);
void timerSetHandler(void *, void (*)(void *), void *);
void timerSetOverflow(void *, uint32_t);
void *timerCreateCapture(void *, struct Gpio, enum captureMode);
void *timerCreatePwm(void *, struct Gpio, uint32_t, uint32_t);
/*----------------------------------------------------------------------------*/
#endif /* TIMER_H_ */
