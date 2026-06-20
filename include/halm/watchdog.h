/*
 * halm/watchdog.h
 * Copyright (C) 2014 xent
 * Project is distributed under the terms of the MIT License
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

  bool (*fired)(const void *);
  void (*reload)(void *);
  void (*setCallback)(void *, void (*)(void *), void *);
};

struct Watchdog
{
  struct Entity base;
};
/*----------------------------------------------------------------------------*/
BEGIN_DECLS

/**
 * Get the watchdog reset status.
 *
 * Checks whether a system reset was triggered by the watchdog timer.
 *
 * @param timer Pointer to a Watchdog object.
 * @return Status of the watchdog reset:
 *   - @b true if the system was reset due to a watchdog timeout.
 *   - @b false if the reset was caused by another source or no reset occurred.
 */
static inline bool watchdogFired(const void *timer)
{
  return ((const struct WatchdogClass *)CLASS(timer))->fired(timer);
}

/**
 * Restart (reload) the watchdog timer.
 *
 * Resets the watchdog's internal counter to its initial value, preventing
 * a system reset. This function must be called periodically during normal
 * operation to avoid a watchdog timeout.
 *
 * @param timer Pointer to a Watchdog object.
 */
static inline void watchdogReload(void *timer)
{
  ((const struct WatchdogClass *)CLASS(timer))->reload(timer);
}

/**
 * Set the watchdog interrupt callback function.
 *
 * Configures a callback function that will be invoked when the watchdog
 * approaches its timeout period in interrupt mode. Not all hardware
 * implementations support interrupt mode.
 *
 * @param timer Pointer to a Watchdog object.
 * @param callback Function pointer to the callback routine. Can be @b NULL
 * to disable the callback.
 * @param argument Pointer to user-defined data that will be passed
 * to the callback function when it is invoked. Can be @b NULL if no context
 * is needed.
 */
static inline void watchdogSetCallback(void *timer, void (*callback)(void *),
    void *argument)
{
  ((const struct WatchdogClass *)CLASS(timer))->setCallback(timer, callback,
      argument);
}

END_DECLS
/*----------------------------------------------------------------------------*/
#endif /* HALM_WATCHDOG_H_ */
