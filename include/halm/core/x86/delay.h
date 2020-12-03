/*
 * halm/core/x86/delay.h
 * Copyright (C) 2018 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_CORE_X86_DELAY_H_
#define HALM_CORE_X86_DELAY_H_
/*----------------------------------------------------------------------------*/
#include <xcore/helpers.h>
#include <stdint.h>
#include <time.h>
/*----------------------------------------------------------------------------*/
BEGIN_DECLS

static inline void mdelay(uint32_t period)
{
  struct timespec requested;

  if (period > 999)
  {
    requested.tv_sec = (time_t)(period / 1000);
    requested.tv_nsec = (period - ((long)requested.tv_sec * 1000)) * 1000000;
  }
  else
  {
    requested.tv_sec = 0;
    requested.tv_nsec = period * 1000000;
  }

  nanosleep(&requested, 0);
}

static inline void udelay(uint32_t period)
{
  struct timespec requested;

  if (period > 999999)
  {
    requested.tv_sec = (time_t)(period / 1000000);
    requested.tv_nsec = (period - ((long)requested.tv_sec * 1000000)) * 1000;
  }
  else
  {
    requested.tv_sec = 0;
    requested.tv_nsec = period * 1000;
  }

  nanosleep(&requested, 0);
}

END_DECLS
/*----------------------------------------------------------------------------*/
#endif /* HALM_CORE_X86_DELAY_H_ */
