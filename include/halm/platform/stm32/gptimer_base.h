/*
 * halm/platform/stm32/gptimer_base.h
 * Copyright (C) 2018 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_STM32_GPTIMER_BASE_H_
#define HALM_PLATFORM_STM32_GPTIMER_BASE_H_
/*----------------------------------------------------------------------------*/
#include <halm/irq.h>
#include <halm/pin.h>
#include <halm/timer.h>
/*----------------------------------------------------------------------------*/
#undef HEADER_PATH
#define HEADER_PATH <halm/platform/PLATFORM_TYPE/PLATFORM/gptimer_base.h>
#include HEADER_PATH
#undef HEADER_PATH
/*----------------------------------------------------------------------------*/
extern const struct EntityClass * const GpTimerBase;

struct GpTimerBaseConfig
{
  /** Mandatory: peripheral identifier. */
  uint8_t channel;
};

struct GpTimerBase
{
  struct Timer base;

  void *reg;
  void (*handler)(void *);
  IrqNumber irq;

  /* Peripheral block identifier */
  uint8_t channel;
  /* Timer resolution in exponential form */
  uint8_t resolution;
};
/*----------------------------------------------------------------------------*/
BEGIN_DECLS

/* Common functions */
int gpTimerAllocateChannel(uint8_t);
uint8_t gpTimerConfigCapturePin(uint8_t, PinNumber, enum PinPull);
uint8_t gpTimerConfigComparePin(uint8_t, PinNumber);

/* Platform-specific functions */
uint32_t gpTimerGetClock(const struct GpTimerBase *);

END_DECLS
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_STM32_GPTIMER_BASE_H_ */
