/*
 * mutex.h
 * Copyright (C) 2012 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef MUTEX_H_
#define MUTEX_H_
/*----------------------------------------------------------------------------*/
#include <stdbool.h>
#include "error.h"
/*----------------------------------------------------------------------------*/
struct Mutex
{
  unsigned char state;
};
/*----------------------------------------------------------------------------*/
enum result mutexInit(struct Mutex *);
void mutexDeinit(struct Mutex *);
/*----------------------------------------------------------------------------*/
void mutexLock(struct Mutex *);
bool mutexTryLock(struct Mutex *);
void mutexUnlock(struct Mutex *);
/*----------------------------------------------------------------------------*/
#endif /* MUTEX_H_ */
