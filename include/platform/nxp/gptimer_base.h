/*
 * platform/nxp/gptimer_base.h
 * Copyright (C) 2013 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef GPTIMER_BASE_H_
#define GPTIMER_BASE_H_
/*----------------------------------------------------------------------------*/
#include <gpio.h>
#include <irq.h>
#include <timer.h>
/*----------------------------------------------------------------------------*/
extern const struct TimerClass *GpTimerBase;
/*----------------------------------------------------------------------------*/
/* Symbolic names for two different types of timers on low-performance parts */
enum gpTimerChannel
{
  TIMER_CT16B0 = 0,
  TIMER_CT16B1,
  TIMER_CT32B0,
  TIMER_CT32B1
};
/*----------------------------------------------------------------------------*/
struct GpTimerBaseConfig
{
  gpio_t input; /* Optional: clock input pin */
  uint8_t channel; /* Mandatory: timer block */
};
/*----------------------------------------------------------------------------*/
struct GpTimerBase
{
  struct Timer parent;

  /* Pointer to Timer/Capture register block */
  void *reg;
  /* Pointer to the interrupt handler */
  void (*handler)(void *);
  /* Interrupt identifier */
  irq_t irq;

  /* External clock input pin */
  struct Gpio input;
  /* Peripheral block identifier */
  uint8_t channel;
};
/*----------------------------------------------------------------------------*/
uint32_t gpTimerGetClock(struct GpTimerBase *);
/*----------------------------------------------------------------------------*/
#endif /* GPTIMER_BASE_H_ */
