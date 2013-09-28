/*
 * mutex.c
 * Copyright (C) 2012 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

//FIXME Rewrite
#include "platform/nxp/device_defs.h"
#include "threading/cortex-m3/mutex.h"
/*----------------------------------------------------------------------------*/
enum mutexState
{
  MUTEX_UNLOCKED = 0,
  MUTEX_LOCKED
};
/*----------------------------------------------------------------------------*/
enum result mutexInit(struct Mutex *m)
{
  m->state = MUTEX_UNLOCKED;
  return E_OK;
}
/*----------------------------------------------------------------------------*/
void mutexDeinit(struct Mutex *m)
{

}
/*----------------------------------------------------------------------------*/
void mutexLock(struct Mutex *m)
{
  do
  {
    /* Wait until mutex becomes free */
    while (__LDREXB(&m->state) != MUTEX_UNLOCKED);
  }
  while (__STREXB(MUTEX_LOCKED, &m->state) != 0); /* Try to set mutex locked */
  __DMB();
}
/*----------------------------------------------------------------------------*/
bool mutexTryLock(struct Mutex *m)
{
  if (__LDREXB(&m->state) == MUTEX_UNLOCKED) /* Get current mutex state */
  {
    /* Try to set mutex locked */
    while (__STREXB(MUTEX_LOCKED, &m->state) != 0);
    __DMB();
    return true;
  }
  return false; /* Mutex is already locked */
}
/*----------------------------------------------------------------------------*/
void mutexUnlock(struct Mutex *m)
{
  __DMB(); /* Ensure memory operations completed before releasing lock */
  m->state = MUTEX_UNLOCKED;
}
