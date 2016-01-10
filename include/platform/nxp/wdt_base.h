/*
 * platform/nxp/wdt_base.h
 * Copyright (C) 2014 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef PLATFORM_NXP_WDT_BASE_H_
#define PLATFORM_NXP_WDT_BASE_H_
/*----------------------------------------------------------------------------*/
#include <entity.h>
#include <irq.h>
/*----------------------------------------------------------------------------*/
#undef HEADER_PATH
#define HEADER_PATH <platform/PLATFORM_TYPE/PLATFORM/wdt_base.h>
#include HEADER_PATH
#undef HEADER_PATH
/*----------------------------------------------------------------------------*/
extern const struct EntityClass * const WdtBase;
/*----------------------------------------------------------------------------*/
struct WdtBaseConfig
{
  enum wdtClockSource source;
};
/*----------------------------------------------------------------------------*/
struct WdtBase
{
  struct Entity base;

  void (*handler)(void *);
  irqNumber irq;
};
/*----------------------------------------------------------------------------*/
uint32_t wdtGetClock(const struct WdtBase *);
/*----------------------------------------------------------------------------*/
#endif /* PLATFORM_NXP_WDT_BASE_H_ */
