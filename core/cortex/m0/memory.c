/*
 * memory.c
 * Copyright (C) 2014 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <memory.h>
/*----------------------------------------------------------------------------*/
bool compareExchangePointer(void **pointer, const void *expected, void *desired)
{
  bool status = false;

  __interruptsDisable();
  /* Compare current state with expected state */
  if (*pointer == expected)
  {
    *pointer = desired;
    status = true;
  }
  __interruptsEnable();

  return status;
}
