/*
 * halm/platform/stm/gptimer_base.h
 * Copyright (C) 2018 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef HALM_PLATFORM_STM_GPTIMER_BASE_H_
#define HALM_PLATFORM_STM_GPTIMER_BASE_H_
/*----------------------------------------------------------------------------*/
#include <halm/irq.h>
#include <halm/timer.h>
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

uint32_t gpTimerGetClock(const struct GpTimerBase *);

END_DECLS
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_STM_GPTIMER_BASE_H_ */
