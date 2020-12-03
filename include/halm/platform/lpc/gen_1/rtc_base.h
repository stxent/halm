/*
 * halm/platform/lpc/gen_1/rtc_base.h
 * Copyright (C) 2015 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_LPC_GEN_1_RTC_BASE_H_
#define HALM_PLATFORM_LPC_GEN_1_RTC_BASE_H_
/*----------------------------------------------------------------------------*/
#include <halm/irq.h>
#include <xcore/realtime.h>
/*----------------------------------------------------------------------------*/
extern const struct EntityClass * const RtcBase;

struct RtcBase
{
  struct RtClock base;

  void *reg;
  void (*handler)(void *);
  IrqNumber irq;
};
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_LPC_GEN_1_RTC_BASE_H_ */
