/*
 * spinlock.c
 * Copyright (C) 2012 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include "platform/nxp/device_defs.h"
#include "platform/nxp/spinlock.h"
/*----------------------------------------------------------------------------*/
void spinLock(spinlock_t *lock)
{
  do
  {
    /* Wait until lock becomes free */
    while (__LDREXB(lock) != SPIN_UNLOCKED);
  }
  while (__STREXB(SPIN_LOCKED, lock) != 0); /* Try to lock */
  __DMB();
}
/*----------------------------------------------------------------------------*/
bool spinTryLock(spinlock_t *lock)
{
  if (__LDREXB(lock) == SPIN_UNLOCKED) /* Get current state */
  {
    /* Try to lock */
    while (__STREXB(SPIN_LOCKED, lock) != 0);
    __DMB();
    return true;
  }
  return false; /* Already locked */
}
/*----------------------------------------------------------------------------*/
void spinUnlock(spinlock_t *lock)
{
  __DMB(); /* Ensure memory operations completed before releasing lock */
  *lock = SPIN_UNLOCKED;
}
