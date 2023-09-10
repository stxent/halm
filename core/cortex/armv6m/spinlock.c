/*
 * spinlock.c
 * Copyright (C) 2013 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/spinlock.h>
#include <xcore/asm.h>
/*----------------------------------------------------------------------------*/
void spinLock(Spinlock *lock)
{
  /* Wait until lock becomes free */
  while (1)
  {
    const uint32_t state = __mrs_primask();
    __cpsid();

    if (*lock == SPIN_UNLOCKED)
    {
      *lock = SPIN_LOCKED;
      __dmb();
      __msr_primask(state);
      return;
    }

    __msr_primask(state);
  }
}
/*----------------------------------------------------------------------------*/
bool spinTryLock(Spinlock *lock)
{
  const uint32_t state = __mrs_primask();
  __cpsid();

  if (*lock == SPIN_UNLOCKED)
  {
    *lock = SPIN_LOCKED;
    __dmb();
    __msr_primask(state);
    return true;
  }

  __msr_primask(state);
  return false; /* Already locked */
}
/*----------------------------------------------------------------------------*/
void spinUnlock(Spinlock *lock)
{
  __dmb(); /* Ensure memory operations completed before releasing lock */
  *lock = SPIN_UNLOCKED;
}
