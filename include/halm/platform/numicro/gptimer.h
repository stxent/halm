/*
 * halm/platform/numicro/gptimer.h
 * Copyright (C) 2023 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_NUMICRO_GPTIMER_H_
#define HALM_PLATFORM_NUMICRO_GPTIMER_H_
/*----------------------------------------------------------------------------*/
#include <halm/platform/numicro/gptimer_base.h>
/*----------------------------------------------------------------------------*/
extern const struct TimerClass * const GpTimer;

struct GpTimerConfig
{
  /**
   * Optional: desired timer tick rate in Hertz. If this field is set to zero,
   * the actual peripheral frequency will be used as the timer tick rate.
   */
  uint32_t frequency;
  /** Optional: interrupt priority for the timer interrupt request. */
  IrqPriority priority;
  /** Mandatory: peripheral identifier number of the timer. */
  uint8_t channel;

  struct
  {
    /** Optional: timer interrupt enables ADC. */
    bool adc;
    /** Optional: timer interrupt enables DAC. */
    bool dac;
    /** Optional: timer interrupt enables PDMA. */
    bool dma;
    /** Optional: timer interrupt enables PWM. */
    bool pwm;
    /** Optional: timer interrupt enables wake-up from Idle and Power-down. */
    bool wakeup;
  } trigger;
};

struct GpTimer
{
  struct GpTimerBase base;

  void (*callback)(void *);
  void *callbackArgument;

  /* Desired timer frequency */
  uint32_t frequency;
};
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_NUMICRO_GPTIMER_H_ */
