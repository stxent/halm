/*
 * spinlock.c
 * Copyright (C) 2012 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <spinlock.h>
#include <core/cortex/m3/asm.h>
/*----------------------------------------------------------------------------*/
void spinLock(spinlock_t *lock)
{
  do
  {
    /* Wait until lock becomes free */
    while (__ldrexb(lock) != SPIN_UNLOCKED);
  }
  while (__strexb(SPIN_LOCKED, lock));

  __dmb();
}
/*----------------------------------------------------------------------------*/
bool spinTryLock(spinlock_t *lock)
{
  if (__ldrexb(lock) == SPIN_UNLOCKED) /* Check current state */
  {
    if (!__strexb(SPIN_LOCKED, lock))
    {
      __dmb();
      return true;
    }
    else
      return false;
  }

  __clrex();
  return false; /* Lock is not free */
}
/*----------------------------------------------------------------------------*/
void spinUnlock(spinlock_t *lock)
{
  __dmb(); /* Ensure memory operations completed before releasing the lock */
  *lock = SPIN_UNLOCKED;
}
