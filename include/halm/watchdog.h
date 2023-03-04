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
  void (*setCallback)(void *, void (*)(void *), void *);
  void (*reload)(void *);
};

struct Watchdog
{
  struct Entity base;
};
/*----------------------------------------------------------------------------*/
BEGIN_DECLS

/**
 * Get a reset status.
 * @param timer Pointer to a Watchdog object.
 * @return Status of the timer, @b true in case of controller reset
 * and @b false otherwise.
 */
static inline bool watchdogFired(const void *timer)
{
  return ((const struct WatchdogClass *)CLASS(timer))->fired(timer);
}

/**
 * Set the interrupt callback.
 * @param timer Pointer to a Watchdog object.
 * @param callback Callback function.
 * @param argument Callback function argument.
 */
static inline void watchdogSetCallback(void *timer, void (*callback)(void *),
    void *argument)
{
  ((const struct WatchdogClass *)CLASS(timer))->setCallback(timer, callback,
      argument);
}

/**
 * Restart the internal timer.
 * @param timer Pointer to a Watchdog object.
 */
static inline void watchdogReload(void *timer)
{
  ((const struct WatchdogClass *)CLASS(timer))->reload(timer);
}

END_DECLS
/*----------------------------------------------------------------------------*/
#endif /* HALM_WATCHDOG_H_ */
