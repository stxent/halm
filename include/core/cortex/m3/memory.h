/*
 * core/cortex/m3/memory.h
 * Copyright (C) 2014 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef MEMORY_H_
#define MEMORY_H_
/*----------------------------------------------------------------------------*/
#include <stdbool.h>
#include "asm.h"
/*----------------------------------------------------------------------------*/
static inline bool compareExchangePointer(void **pointer, void *expected,
    void *desired)
{
  /* Compare current state with expected state */
  if (__ldrex((uint32_t *)pointer) == (uint32_t)expected)
  {
    if (!__strex((uint32_t)desired, (uint32_t *)pointer))
    {
      __dmb();
      return true;
    }
    else
      return false;
  }

  /* Value has changed outside current context */
  __clrex();
  return false;
}
/*----------------------------------------------------------------------------*/
#endif /* MEMORY_H_ */
