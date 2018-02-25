/*
 * spinlock.c
 * Copyright (C) 2012 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <xcore/core/cortex/asm.h>
#include <halm/spinlock.h>
/*----------------------------------------------------------------------------*/
void spinLock(Spinlock *lock)
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
bool spinTryLock(Spinlock *lock)
{
  if (__ldrexb(lock) == SPIN_UNLOCKED) /* Check current state */
  {
    if (!__strexb(SPIN_LOCKED, lock))
    {
      __dmb();
      return true;
    }
  }

  __clrex();
  return false; /* Lock is not free */
}
/*----------------------------------------------------------------------------*/
void spinUnlock(Spinlock *lock)
{
  __dmb(); /* Ensure memory operations completed before releasing the lock */
  *lock = SPIN_UNLOCKED;
}
