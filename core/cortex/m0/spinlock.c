/*
 * spinlock.c
 * Copyright (C) 2013 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <core/cortex/m0/asm.h>
#include <spinlock.h>
/*----------------------------------------------------------------------------*/
void spinLock(spinlock_t *lock)
{
  /* Wait until lock becomes free */
  while (1)
  {
    __irqDisable();
    if (*lock == SPIN_UNLOCKED) /* Check current state */
    {
      *lock = SPIN_LOCKED; /* Lock */
      __dmb();
      __irqEnable();
      return;
    }
    __irqEnable();
  }
}
/*----------------------------------------------------------------------------*/
bool spinTryLock(spinlock_t *lock)
{
  __irqDisable();
  if (*lock == SPIN_UNLOCKED) /* Check current state */
  {
    *lock = SPIN_LOCKED; /* Lock */
    __dmb();
    __irqEnable();
    return true;
  }
  __irqEnable();
  return false; /* Already locked */
}
/*----------------------------------------------------------------------------*/
void spinUnlock(spinlock_t *lock)
{
  __dmb(); /* Ensure memory operations completed before releasing lock */
  *lock = SPIN_UNLOCKED;
}
