/*
 * platform/nxp/gptimer_base.h
 * Copyright (C) 2013 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef PLATFORM_NXP_GPTIMER_BASE_H_
#define PLATFORM_NXP_GPTIMER_BASE_H_
/*----------------------------------------------------------------------------*/
#include <irq.h>
#include <pin.h>
#include <timer.h>
/*----------------------------------------------------------------------------*/
extern const struct EntityClass * const GpTimerBase;
/*----------------------------------------------------------------------------*/
/** Symbolic names for two different types of timers. */
enum
{
  GPTIMER_CT16B0,
  GPTIMER_CT16B1,
  GPTIMER_CT32B0,
  GPTIMER_CT32B1
};
/*----------------------------------------------------------------------------*/
enum gpTimerEvent
{
  GPTIMER_MATCH_AUTO,
  GPTIMER_MATCH0,
  GPTIMER_MATCH1,
  GPTIMER_MATCH2,
  GPTIMER_MATCH3,
  GPTIMER_EVENT_END
};
/*----------------------------------------------------------------------------*/
struct GpTimerBaseConfig
{
  /** Mandatory: peripheral identifier. */
  uint8_t channel;
};
/*----------------------------------------------------------------------------*/
struct GpTimerBase
{
  struct Timer parent;

  void *reg;
  void (*handler)(void *);
  irq_t irq;

  /* External clock input pin */
  struct Pin input;
  /* Peripheral block identifier */
  uint8_t channel;
  /* Exponent of the timer resolution */
  uint8_t resolution;
};
/*----------------------------------------------------------------------------*/
int8_t gpTimerAllocateChannel(uint8_t);
int8_t gpTimerConfigCapturePin(uint8_t, pin_t, enum pinPull);
int8_t gpTimerConfigMatchPin(uint8_t, pin_t);

uint32_t gpTimerGetClock(const struct GpTimerBase *);
/*----------------------------------------------------------------------------*/
#endif /* PLATFORM_NXP_GPTIMER_BASE_H_ */
