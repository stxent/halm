/*
 * halm/platform/lpc/gptimer_counter.h
 * Copyright (C) 2016 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_LPC_GPTIMER_COUNTER_H_
#define HALM_PLATFORM_LPC_GPTIMER_COUNTER_H_
/*----------------------------------------------------------------------------*/
#include <halm/capture.h>
#include <halm/platform/lpc/gptimer_base.h>
/*----------------------------------------------------------------------------*/
extern const struct TimerClass * const GpTimerCounter;

struct GpTimerCounterConfig
{
  /** Mandatory: active edge. */
  enum InputEvent edge;
  /** Mandatory: pin used as an input. */
  PinNumber pin;
  /** Optional: match event used as a reset source for the timer. */
  enum GpTimerEvent event;
  /** Optional: timer interrupt priority. */
  IrqPriority priority;
  /** Mandatory: peripheral identifier. */
  uint8_t channel;
};

struct GpTimerCounter
{
  struct GpTimerBase base;

  void (*callback)(void *);
  void *callbackArgument;

  /* Match channel used for counter reset */
  uint8_t event;
};
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_LPC_GPTIMER_COUNTER_H_ */
