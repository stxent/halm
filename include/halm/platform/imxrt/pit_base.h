/*
 * halm/platform/imxrt/pit_base.h
 * Copyright (C) 2024 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_IMXRT_PIT_BASE_H_
#define HALM_PLATFORM_IMXRT_PIT_BASE_H_
/*----------------------------------------------------------------------------*/
#include <halm/irq.h>
#include <halm/pin.h>
#include <halm/timer.h>
/*----------------------------------------------------------------------------*/
extern const struct EntityClass * const PitBase;

struct PitBaseConfig
{
  /** Mandatory: peripheral identifier. */
  uint8_t channel;
  /** Optional: enable chaining mode, chain next timer with current one. */
  bool chain;
};

struct PitBase
{
  struct Timer base;

  void *reg;
  void (*handler)(void *);
  IrqNumber irq;

  /* Base timer number */
  uint8_t channel;
  /* Timer used as an overflow counter */
  uint8_t counter;
  /* Chain mode enabled */
  bool chain;
};
/*----------------------------------------------------------------------------*/
BEGIN_DECLS

/* Common functions */
void pitConfigTriggerPin(uint8_t, PinNumber);

/* Platform-specific functions */
uint32_t pitGetClock(const struct PitBase *);

END_DECLS
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_IMXRT_PIT_BASE_H_ */
