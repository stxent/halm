/*
 * halm/platform/lpc/wwdt.h
 * Copyright (C) 2023 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_NXP_WWDT_H_
#define HALM_PLATFORM_NXP_WWDT_H_
/*----------------------------------------------------------------------------*/
#include <halm/platform/lpc/wdt_base.h>
#include <halm/watchdog.h>
/*----------------------------------------------------------------------------*/
extern const struct WatchdogClass * const Wwdt;

struct WwdtConfig
{
  /** Mandatory: timer period in milliseconds. */
  uint32_t period;
  /**
   * Optional: timer window in milliseconds. Should be set to zero in the
   * periodic interrupt mode.
   */
  uint32_t window;
  /** Optional: timer clock source. */
  enum WdtClockSource source;
   /** Optional: timer interrupt priority. */
  IrqPriority priority;
  /** Optional: use periodic interrupt mode and disable controller reset. */
  bool disarmed;
};

struct Wwdt
{
  struct WdtBase base;

  void (*callback)(void *);
  void *callbackArgument;

  /* Watchdog reset occurred */
  bool fired;
};
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_NXP_WWDT_H_ */
