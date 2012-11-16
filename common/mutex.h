/*
 * mutex.h
 *
 *  Created on: Oct 20, 2012
 *      Author: xen
 */

#ifndef MUTEX_H_
#define MUTEX_H_
/*----------------------------------------------------------------------------*/
#include <stdint.h>
/*----------------------------------------------------------------------------*/
#include "entity.h"
/*----------------------------------------------------------------------------*/
struct Mutex
{
  uint8_t state;
};
/*----------------------------------------------------------------------------*/
enum result mutexInit(struct Mutex *);
void mutexDeinit(struct Mutex *);
void mutexLock(struct Mutex *);
uint8_t mutexTryLock(struct Mutex *);
void mutexUnlock(struct Mutex *);
/*----------------------------------------------------------------------------*/
#endif /* MUTEX_H_ */
