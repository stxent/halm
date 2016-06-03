/*
 * spinlock.c
 * Copyright (C) 2013 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <xcore/core/cortex/m0/asm.h>
#include <halm/spinlock.h>
/*----------------------------------------------------------------------------*/
void spinLock(spinlock_t *lock)
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
bool spinTryLock(spinlock_t *lock)
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
void spinUnlock(spinlock_t *lock)
{
  __dmb(); /* Ensure memory operations completed before releasing lock */
  *lock = SPIN_UNLOCKED;
}
