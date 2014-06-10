/*
 * clock.h
 * Copyright (C) 2013 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

/**
 * @file
 * Abstract class for clock subsystem components.
 */

#ifndef CLOCK_H_
#define CLOCK_H_
/*----------------------------------------------------------------------------*/
#include <stdint.h>
#include <stdbool.h>
#include <error.h>
/*----------------------------------------------------------------------------*/
/* Simplified descriptor for abstract class */
struct ClockClass
{
  enum result (*disable)(void);
  enum result (*enable)(const void *);
  uint32_t (*frequency)(void);
  bool (*ready)(void);
};
/*----------------------------------------------------------------------------*/
/**
 * Try to stop specified clock.
 * @param clock Class descriptor.
 * @return @b E_OK on success.
 */
static inline enum result clockDisable(const void *clock)
{
  return ((struct ClockClass *)clock)->disable();
}
/*----------------------------------------------------------------------------*/
/**
 * Start or restart clock with specified parameters.
 * @param clock Class descriptor.
 * @param config Clock configuration data, in some cases may be zero.
 * @return @b E_OK on success.
 */
static inline enum result clockEnable(const void *clock, const void *config)
{
  return ((struct ClockClass *)clock)->enable(config);
}
/*----------------------------------------------------------------------------*/
/**
 * Get clock frequency.
 * @param clock Class descriptor.
 * @return Clock frequency on success or zero otherwise.
 */
static inline uint32_t clockFrequency(const void *clock)
{
  return ((struct ClockClass *)clock)->frequency();
}
/*----------------------------------------------------------------------------*/
/**
 * Check whether clock is ready or not.
 * @param clock Class descriptor.
 * @return @b true when clock is ready to be used or @b false otherwise.
 */
static inline bool clockReady(const void *clock)
{
  return ((struct ClockClass *)clock)->ready();
}
/*----------------------------------------------------------------------------*/
#endif /* CLOCK_H_ */
