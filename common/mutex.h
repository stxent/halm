/*
 * mutex.h
 *
 *  Created on: Oct 20, 2012
 *      Author: xen
 */

#ifndef MUTEX_H_
#define MUTEX_H_
/*------------------------------------------------------------------------------*/
#include <stdint.h>
/*------------------------------------------------------------------------------*/
struct Mutex
{
  uint8_t state;
};
/*------------------------------------------------------------------------------*/
void mutexLock(struct Mutex *);
uint8_t mutexTryLock(struct Mutex *);
void mutexRelease(struct Mutex *);
/*------------------------------------------------------------------------------*/
#endif /* MUTEX_H_ */
