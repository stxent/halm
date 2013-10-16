/*
 * clock.c
 * Copyright (C) 2013 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <clock.h>
/*----------------------------------------------------------------------------*/
/**
 * Try to stop specified clock.
 * @param clock Class descriptor.
 */
void clockDisable(const void *clock)
{
  ((struct ClockClass *)clock)->disable();
}
/*----------------------------------------------------------------------------*/
/**
 * Start or restart clock with specified parameters.
 * @param clock Class descriptor.
 * @param config Clock configuration data, in some cases may be zero.
 * @return @b E_OK on success.
 */
enum result clockEnable(const void *clock, const void *config)
{
  return ((struct ClockClass *)clock)->enable(config);
}
/*----------------------------------------------------------------------------*/
/**
 * Check whether clock is ready or not.
 * @param clock Class descriptor.
 * @return @b true when clock is ready to be used or @b false otherwise.
 */
bool clockReady(const void *clock)
{
  return ((struct ClockClass *)clock)->ready();
}
