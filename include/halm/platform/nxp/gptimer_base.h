/*
 * halm/platform/nxp/gptimer_base.h
 * Copyright (C) 2013 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef HALM_PLATFORM_NXP_GPTIMER_BASE_H_
#define HALM_PLATFORM_NXP_GPTIMER_BASE_H_
/*----------------------------------------------------------------------------*/
#include <halm/irq.h>
#include <halm/pin.h>
#include <halm/timer.h>
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
enum GpTimerEvent
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
  struct Timer base;

  void *reg;
  void (*handler)(void *);
  IrqNumber irq;

  /* External clock input pin */
  struct Pin input;
  /* Peripheral block identifier */
  uint8_t channel;
  /* Exponent of the timer resolution */
  uint8_t resolution;
};
/*----------------------------------------------------------------------------*/
int gpTimerAllocateChannel(uint8_t);
uint8_t gpTimerConfigCapturePin(uint8_t, PinNumber, enum PinPull);
uint8_t gpTimerConfigMatchPin(uint8_t, PinNumber);

uint32_t gpTimerGetClock(const struct GpTimerBase *);
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_NXP_GPTIMER_BASE_H_ */
