/*
 * clock.h
 * Copyright (C) 2013 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

/**
 * @file
 * Abstract class for clock subsystem components.
 */

#ifndef HALM_CLOCK_H_
#define HALM_CLOCK_H_
/*----------------------------------------------------------------------------*/
#include <stdint.h>
#include <stdbool.h>
#include <error.h>
/*----------------------------------------------------------------------------*/
/* Simplified descriptor for abstract class */
struct ClockClass
{
  void (*disable)(const void *);
  enum result (*enable)(const void *, const void *);
  uint32_t (*frequency)(const void *);
  bool (*ready)(const void *);
};
/*----------------------------------------------------------------------------*/
/**
 * Stop specified clock source.
 * @param clock Class descriptor.
 */
static inline void clockDisable(const void *clock)
{
  ((const struct ClockClass *)clock)->disable(clock);
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
  return ((const struct ClockClass *)clock)->enable(clock, config);
}
/*----------------------------------------------------------------------------*/
/**
 * Get clock frequency.
 * @param clock Class descriptor.
 * @return Clock frequency on success or zero otherwise.
 */
static inline uint32_t clockFrequency(const void *clock)
{
  return ((const struct ClockClass *)clock)->frequency(clock);
}
/*----------------------------------------------------------------------------*/
/**
 * Check whether clock is ready or not.
 * @param clock Class descriptor.
 * @return @b true when clock is ready to be used or @b false otherwise.
 */
static inline bool clockReady(const void *clock)
{
  return ((const struct ClockClass *)clock)->ready(clock);
}
/*----------------------------------------------------------------------------*/
#endif /* HALM_CLOCK_H_ */
