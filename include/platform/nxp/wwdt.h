/*
 * wwdt.h
 * Copyright (C) 2014 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef PLATFORM_NXP_WWDT_H_
#define PLATFORM_NXP_WWDT_H_
/*----------------------------------------------------------------------------*/
#include <watchdog.h>
#include <platform/nxp/wwdt_base.h>
/*----------------------------------------------------------------------------*/
extern const struct WatchdogClass * const Wwdt;
/*----------------------------------------------------------------------------*/
struct WwdtConfig
{
  /** Watchdog timer period in milliseconds. */
  uint32_t period;
  /** Optional: interrupt priority. */
  priority_t priority;
};
/*----------------------------------------------------------------------------*/
struct Wwdt
{
  struct WwdtBase parent;

  void (*callback)(void *);
  void *callbackArgument;
};
/*----------------------------------------------------------------------------*/
#endif /* PLATFORM_NXP_WWDT_H_ */
