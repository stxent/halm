/*
 * mutex.c
 *
 *  Created on: Oct 20, 2012
 *      Author: xen
 */

#include "mutex.h"
/*------------------------------------------------------------------------------*/
#define MUTEX_FREE      0
#define MUTEX_LOCKED    1
/*------------------------------------------------------------------------------*/
void mutexLock(struct Mutex *m)
{
  while (m->state != MUTEX_FREE);
  m->state = MUTEX_LOCKED;
}
/*------------------------------------------------------------------------------*/
uint8_t mutexTryLock(struct Mutex *m)
{
  if (m->state)
  {
    m->state = MUTEX_LOCKED;
    return 0; /* Operation successful */
  }
  else
    return 1; /* Error: mutex already locked */
}
/*------------------------------------------------------------------------------*/
void mutexUnlock(struct Mutex *m)
{
  m->state = MUTEX_FREE;
}
