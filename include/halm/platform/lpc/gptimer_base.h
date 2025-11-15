/*
 * halm/platform/lpc/gptimer_base.h
 * Copyright (C) 2013 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_LPC_GPTIMER_BASE_H_
#define HALM_PLATFORM_LPC_GPTIMER_BASE_H_
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
enum [[gnu::packed]] GpTimerEvent
{
  GPTIMER_MATCH_AUTO,
  GPTIMER_MATCH0,
  GPTIMER_MATCH1,
  GPTIMER_MATCH2,
  GPTIMER_MATCH3,
  GPTIMER_EVENT_END
};

enum GpTimerFlags
{
  GPTIMER_FLAG_32_BIT = 0x01
};
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

  /* External clock input pin */
  struct Pin input;
  /* Peripheral block identifier */
  uint8_t channel;
  /* Peripheral block capabilities */
  uint8_t flags;
};
/*----------------------------------------------------------------------------*/
BEGIN_DECLS

/* Common functions */
int gpTimerAllocateChannel(uint8_t);
uint8_t gpTimerConfigCapturePin(uint8_t, PinNumber, enum PinPull);
uint8_t gpTimerConfigMatchPin(uint8_t, PinNumber, bool);
uint8_t gpTimerGetMatchChannel(uint8_t, PinNumber);
void gpTimerSetFrequency(struct GpTimerBase *, uint32_t);

/* Platform-specific functions */
uint32_t gpTimerGetClock(const struct GpTimerBase *);

END_DECLS
/*----------------------------------------------------------------------------*/
BEGIN_DECLS

static inline uint32_t gpTimerGetMaxValue(const struct GpTimerBase *timer)
{
  return (timer->flags & GPTIMER_FLAG_32_BIT) ? 0xFFFFFFFFUL : 0xFFFFUL;
}

END_DECLS
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_LPC_GPTIMER_BASE_H_ */
