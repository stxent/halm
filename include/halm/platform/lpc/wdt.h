/*
 * halm/platform/lpc/wdt.h
 * Copyright (C) 2014 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_LPC_WDT_H_
#define HALM_PLATFORM_LPC_WDT_H_
/*----------------------------------------------------------------------------*/
#include <halm/platform/lpc/wdt_base.h>
#include <halm/watchdog.h>
/*----------------------------------------------------------------------------*/
extern const struct WatchdogClass * const Wdt;

struct WdtConfig
{
  /** Mandatory: timer period in milliseconds. */
  uint32_t period;
  /** Optional: timer clock source. */
  enum WdtClockSource source;
};

struct Wdt
{
  struct WdtBase base;

  /* Watchdog reset occurred */
  bool fired;
};
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_LPC_WDT_H_ */
