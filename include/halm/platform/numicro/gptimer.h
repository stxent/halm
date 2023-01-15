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
   * Optional: desired timer tick rate. An actual peripheral frequency is used
   * when option is set to zero.
   */
  uint32_t frequency;
  /** Optional: timer interrupt priority. */
  IrqPriority priority;
  /** Mandatory: peripheral identifier. */
  uint8_t channel;

  struct
  {
    /** Optional: timer interrupt enables ADC. */
    bool adc;
    /** Optional: timer interrupt enables BPWM. */
    bool bpwm;
    /** Optional: timer interrupt enables PDMA. */
    bool pdma;
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
