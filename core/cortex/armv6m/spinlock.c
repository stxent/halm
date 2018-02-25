/*
 * spinlock.c
 * Copyright (C) 2013 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <xcore/core/cortex/asm.h>
#include <halm/spinlock.h>
/*----------------------------------------------------------------------------*/
void spinLock(Spinlock *lock)
{
  /* Wait until lock becomes free */
  while (1)
  {
    __interruptsDisable();
    if (*lock == SPIN_UNLOCKED)
    {
      *lock = SPIN_LOCKED;
      __dmb();
      __interruptsEnable();
      return;
    }
    __interruptsEnable();
  }
}
/*----------------------------------------------------------------------------*/
bool spinTryLock(Spinlock *lock)
{
  __interruptsDisable();
  if (*lock == SPIN_UNLOCKED)
  {
    *lock = SPIN_LOCKED;
    __dmb();
    __interruptsEnable();
    return true;
  }
  __interruptsEnable();
  return false; /* Already locked */
}
/*----------------------------------------------------------------------------*/
void spinUnlock(Spinlock *lock)
{
  __dmb(); /* Ensure memory operations completed before releasing lock */
  *lock = SPIN_UNLOCKED;
}
