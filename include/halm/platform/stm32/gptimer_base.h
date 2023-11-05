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

enum GpTimerEvent
{
  TIM_EVENT_DISABLED,
  TIM_EVENT_UPDATE,
  TIM_EVENT_CC1,
  TIM_EVENT_CC2,
  TIM_EVENT_CC3,
  TIM_EVENT_CC4
} __attribute__((packed));

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
  /* Peripheral block capabilities */
  uint8_t flags;
};
/*----------------------------------------------------------------------------*/
BEGIN_DECLS

/* Common functions */
int gpTimerAllocateChannel(uint8_t);
uint8_t gpTimerConfigInputPin(uint8_t, PinNumber, enum PinPull);
uint8_t gpTimerConfigOutputPin(uint8_t, PinNumber);
void gpTimerSetTimerFrequency(struct GpTimerBase *, uint32_t);

/* Platform-specific functions */
uint32_t gpTimerGetClock(const struct GpTimerBase *);

END_DECLS
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_STM32_GPTIMER_BASE_H_ */
