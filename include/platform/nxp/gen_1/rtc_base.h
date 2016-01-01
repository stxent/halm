/*
 * platform/nxp/gen_1/rtc_base.h
 * Copyright (C) 2015 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef PLATFORM_NXP_GEN_1_RTC_BASE_H_
#define PLATFORM_NXP_GEN_1_RTC_BASE_H_
/*----------------------------------------------------------------------------*/
#include <irq.h>
#include <pin.h>
#include <realtime.h>
/*----------------------------------------------------------------------------*/
extern const struct EntityClass * const RtcBase;
/*----------------------------------------------------------------------------*/
struct RtcBase
{
  struct RtClock parent;

  void *reg;
  void (*handler)(void *);
  irqNumber irq;
};
/*----------------------------------------------------------------------------*/
#endif /* PLATFORM_NXP_GEN_1_RTC_BASE_H_ */
