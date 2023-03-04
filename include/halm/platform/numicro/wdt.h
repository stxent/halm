/*
 * halm/platform/numicro/wdt.h
 * Copyright (C) 2023 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_NUMICRO_WDT_H_
#define HALM_PLATFORM_NUMICRO_WDT_H_
/*----------------------------------------------------------------------------*/
#include <halm/platform/numicro/wdt_base.h>
#include <halm/watchdog.h>
/*----------------------------------------------------------------------------*/
extern const struct WatchdogClass * const Wdt;

struct WdtConfig
{
  /** Mandatory: timer period in milliseconds. */
  uint32_t period;
  /** Optional: timer interrupt priority. */
  IrqPriority priority;
  /** Optional: disable controller reset. */
  bool disarmed;
};

struct Wdt
{
  struct WdtBase base;

  void (*callback)(void *);
  void *callbackArgument;

  /* Watchdog reset occurred */
  bool fired;
};
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_NUMICRO_WDT_H_ */
