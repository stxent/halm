/*
 * base_timer.h
 * Copyright (C) 2013 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef BASE_TIMER_H_
#define BASE_TIMER_H_
/*----------------------------------------------------------------------------*/
#include "platform/gpio.h"
#include "platform/timer.h"
#include "../device_defs.h"
#include "../nvic.h"
/*----------------------------------------------------------------------------*/
extern const struct TimerClass *BaseTimer;
/*----------------------------------------------------------------------------*/
/* Registers are common for two timer types with different resolutions */
/* To distinguish them all timers/counter have their own symbolic identifiers */
enum baseTimerNumber
{
  TIMER_CT16B0 = 0,
  TIMER_CT16B1,
  TIMER_CT32B0,
  TIMER_CT32B1
};
/*----------------------------------------------------------------------------*/
struct BaseTimerConfig
{
  uint32_t frequency; /* Mandatory: timer fundamental frequency */
  gpioKey input; /* Optional: clock input pin */
  uint8_t channel; /* Mandatory: timer block */
};
/*----------------------------------------------------------------------------*/
struct BaseTimer
{
  struct Timer parent;

  void (*handler)(void *); /* Hardware interrupt handler */
  void (*callback)(void *); /* User interrupt handler */
  void *callbackArgument; /* User interrupt handler argument */

  LPC_TMR_TypeDef *reg; /* Pointer to UART registers */
  irq_t irq; /* IRQ identifier */

  struct Gpio input; /* External clock input pin */
  uint8_t channel; /* Identifier of peripheral block for internal use */
};
/*----------------------------------------------------------------------------*/
#endif /* BASE_TIMER_H_ */
