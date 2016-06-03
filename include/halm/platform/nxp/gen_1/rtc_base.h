/*
 * halm/platform/nxp/gen_1/rtc_base.h
 * Copyright (C) 2015 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef HALM_PLATFORM_NXP_GEN_1_RTC_BASE_H_
#define HALM_PLATFORM_NXP_GEN_1_RTC_BASE_H_
/*----------------------------------------------------------------------------*/
#include <xcore/realtime.h>
#include <halm/irq.h>
#include <halm/pin.h>
/*----------------------------------------------------------------------------*/
extern const struct EntityClass * const RtcBase;
/*----------------------------------------------------------------------------*/
struct RtcBase
{
  struct RtClock base;

  void *reg;
  void (*handler)(void *);
  irqNumber irq;
};
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_NXP_GEN_1_RTC_BASE_H_ */
