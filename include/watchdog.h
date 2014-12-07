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
#include <stdbool.h>
#include <stdint.h>
#include <entity.h>
/*----------------------------------------------------------------------------*/
/* Class descriptor */
struct WatchdogClass
{
  CLASS_HEADER

  void (*restart)(void *);
};
/*----------------------------------------------------------------------------*/
struct Watchdog
{
  struct Entity parent;
};
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
