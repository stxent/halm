/*
 * mutex.c
 * Copyright (C) 2012 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <LPC13xx.h>
#include "mutex.h"
/*----------------------------------------------------------------------------*/
void mutexLock(Mutex *m)
{
  do
  {
    while (__LDREXB(m) != MUTEX_UNLOCKED); /* Wait until mutex becomes free */
  }
  while (__STREXB(MUTEX_LOCKED, m) != 0); /* Try to set mutex locked */
  __DMB(); /* Data memory barrier instruction */
}
/*----------------------------------------------------------------------------*/
bool mutexTryLock(Mutex *m)
{
  if (__LDREXB(m) == MUTEX_UNLOCKED) /* Get current mutex state */
  {
    while (__STREXB(MUTEX_LOCKED, m) != 0); /* Try to set mutex locked */
    __DMB(); /* Data memory barrier instruction */
    return true;
  }
  return false; /* Mutex is already locked */
}
/*----------------------------------------------------------------------------*/
void mutexUnlock(Mutex *m)
{
  __DMB(); /* Ensure memory operations completed before releasing lock */
  *m = MUTEX_UNLOCKED;
}
