/*
 * platform/nxp/gptimer_base.h
 * Copyright (C) 2013 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef GPTIMER_BASE_H_
#define GPTIMER_BASE_H_
/*----------------------------------------------------------------------------*/
#include <irq.h>
#include <pin.h>
#include <timer.h>
/*----------------------------------------------------------------------------*/
extern const struct EntityClass * const GpTimerBase;
/*----------------------------------------------------------------------------*/
struct GpTimerBaseConfig
{
  uint8_t channel; /* Mandatory: timer block */
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
};
/*----------------------------------------------------------------------------*/
int8_t gpTimerAllocateChannel(uint8_t);
int8_t gpTimerSetupCapturePin(uint8_t, pin_t);
int8_t gpTimerSetupMatchPin(uint8_t, pin_t);

uint32_t gpTimerGetClock(struct GpTimerBase *);
/*----------------------------------------------------------------------------*/
#endif /* GPTIMER_BASE_H_ */
