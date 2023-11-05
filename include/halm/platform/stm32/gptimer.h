/*
 * halm/platform/stm32/gptimer.h
 * Copyright (C) 2019 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_STM32_GPTIMER_H_
#define HALM_PLATFORM_STM32_GPTIMER_H_
/*----------------------------------------------------------------------------*/
#include <halm/platform/stm32/gptimer_base.h>
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
  /**
   * Optional: timer event for DMA request and internal request generation.
   * Timer may support only a part of possible capture/compare events, timer
   * capabilities should be checked in the datasheet.
   * Zero disables the generation of the request.
   */
  enum GpTimerEvent event;
};

struct GpTimer
{
  struct GpTimerBase base;

  void (*callback)(void *);
  void *callbackArgument;

  /* Desired timer frequency */
  uint32_t frequency;
  /* Event used for a DMA request generation */
  enum GpTimerEvent event;
};
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_STM32_GPTIMER_H_ */
