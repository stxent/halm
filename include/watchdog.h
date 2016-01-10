/*
 * watchdog.h
 * Copyright (C) 2014 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

/**
 * @file
 * Abstract watchdog class.
 */

#ifndef WATCHDOG_H_
#define WATCHDOG_H_
/*----------------------------------------------------------------------------*/
#include <entity.h>
/*----------------------------------------------------------------------------*/
/* Class descriptor */
struct WatchdogClass
{
  CLASS_HEADER

  void (*callback)(void *, void (*)(void *), void *);
  void (*restart)(void *);
};
/*----------------------------------------------------------------------------*/
struct Watchdog
{
  struct Entity base;
};
/*----------------------------------------------------------------------------*/
/**
 * Set watchdog interrupt callback.
 * @param timer Pointer to a Watchdog object.
 * @param callback Callback function.
 * @param argument Callback function argument.
 */
static inline void watchdogCallback(void *timer, void (*callback)(void *),
    void *argument)
{
  ((const struct WatchdogClass *)CLASS(timer))->callback(timer,
      callback, argument);
}
/*----------------------------------------------------------------------------*/
/**
 * Restart internal timer.
 * @param timer Pointer to a Watchdog object.
 */
static inline void watchdogRestart(void *timer)
{
  ((const struct WatchdogClass *)CLASS(timer))->restart(timer);
}
/*----------------------------------------------------------------------------*/
#endif /* WATCHDOG_H_ */
