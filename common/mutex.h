/*
 * mutex.h
 * Copyright (C) 2012 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef MUTEX_H_
#define MUTEX_H_
/*----------------------------------------------------------------------------*/
#include <stdint.h>
#include <stdbool.h>
/*----------------------------------------------------------------------------*/
#include "error.h"
/*----------------------------------------------------------------------------*/
enum mutexState {
  MUTEX_UNLOCKED = 0,
  MUTEX_LOCKED
};
/*----------------------------------------------------------------------------*/
typedef volatile uint8_t Mutex;
/*----------------------------------------------------------------------------*/
void mutexLock(Mutex *);
bool mutexTryLock(Mutex *);
void mutexUnlock(Mutex *);
/*----------------------------------------------------------------------------*/
#endif /* MUTEX_H_ */
