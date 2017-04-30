/*
 * halm/watchdog.h
 * Copyright (C) 2014 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

/**
 * @file
 * Abstract watchdog class.
 */

#ifndef HALM_WATCHDOG_H_
#define HALM_WATCHDOG_H_
/*----------------------------------------------------------------------------*/
#include <xcore/entity.h>
/*----------------------------------------------------------------------------*/
/* Class descriptor */
struct WatchdogClass
{
  CLASS_HEADER

  enum Result (*setCallback)(void *, void (*)(void *), void *);
  void (*reload)(void *);
};
/*----------------------------------------------------------------------------*/
struct Watchdog
{
  struct Entity base;
};
/*----------------------------------------------------------------------------*/
/**
 * Set the interrupt callback.
 * @param timer Pointer to a Watchdog object.
 * @param callback Callback function.
 * @param argument Callback function argument.
 */
static inline enum Result watchdogSetCallback(void *timer,
    void (*callback)(void *), void *argument)
{
  return ((const struct WatchdogClass *)CLASS(timer))->setCallback(timer,
      callback, argument);
}
/*----------------------------------------------------------------------------*/
/**
 * Restart the internal timer.
 * @param timer Pointer to a Watchdog object.
 */
static inline void watchdogReload(void *timer)
{
  ((const struct WatchdogClass *)CLASS(timer))->reload(timer);
}
/*----------------------------------------------------------------------------*/
#endif /* HALM_WATCHDOG_H_ */
