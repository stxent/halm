/*
 * platform/nxp/wdt.h
 * Copyright (C) 2014 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef PLATFORM_NXP_WDT_H_
#define PLATFORM_NXP_WDT_H_
/*----------------------------------------------------------------------------*/
#include <platform/nxp/wdt_base.h>
#include <watchdog.h>
/*----------------------------------------------------------------------------*/
extern const struct WatchdogClass * const Wdt;
/*----------------------------------------------------------------------------*/
struct WdtConfig
{
  /** Watchdog timer period in milliseconds. */
  uint32_t period;
  /** Optional: timer clock source. */
  enum wdtClockSource source;
  /** Optional: interrupt priority. */
  irqPriority priority;
};
/*----------------------------------------------------------------------------*/
struct Wdt
{
  struct WdtBase base;

  void (*callback)(void *);
  void *callbackArgument;
};
/*----------------------------------------------------------------------------*/
#endif /* PLATFORM_NXP_WDT_H_ */
