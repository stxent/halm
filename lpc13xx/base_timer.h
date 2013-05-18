/*
 * base_timer.h
 * Copyright (C) 2013 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef BASE_TIMER_H_
#define BASE_TIMER_H_
/*----------------------------------------------------------------------------*/
#include <LPC13xx.h>
#include "gpio.h"
#include "timer.h"
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
  uint32_t frequency;
  uint8_t channel;
};
/*----------------------------------------------------------------------------*/
struct BaseTimer
{
  struct Timer parent;

  void (*handler)(void *); /* Hardware interrupt handler */
  void (*callback)(void *); /* User interrupt handler */
  void *callbackParameters; /* User interrupt handler parameters */

  LPC_TMR_TypeDef *reg; /* Pointer to UART registers */
  IRQn_Type irq; /* IRQ identifier */

  uint8_t channel; /* Identifier of peripheral block for internal use */
};
/*----------------------------------------------------------------------------*/
enum result btSetDescriptor(uint8_t, void *);
/*----------------------------------------------------------------------------*/
#endif /* BASE_TIMER_H_ */
