/*
 * memory.c
 * Copyright (C) 2014 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <memory.h>
/*----------------------------------------------------------------------------*/
bool compareExchangePointer(void **pointer, const void *expected, void *desired)
{
  /* Compare current state with expected state */
  if (__ldrex((volatile uint32_t *)pointer) == (uint32_t)expected)
  {
    if (!__strex((uint32_t)desired, (volatile uint32_t *)pointer))
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
