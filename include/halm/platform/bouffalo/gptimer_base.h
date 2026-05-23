/*
 * halm/platform/bouffalo/gptimer_base.h
 * Copyright (C) 2026 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_BOUFFALO_GPTIMER_BASE_H_
#define HALM_PLATFORM_BOUFFALO_GPTIMER_BASE_H_
/*----------------------------------------------------------------------------*/
#include <halm/irq.h>
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

  void (*handler)(void *);
  IrqNumber irq;

  /* Peripheral block identifier */
  uint8_t channel;
};
/*----------------------------------------------------------------------------*/
BEGIN_DECLS

/* Platform-specific functions */
uint32_t gpTimerGetClock(const struct GpTimerBase *);

END_DECLS
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_BOUFFALO_GPTIMER_BASE_H_ */
