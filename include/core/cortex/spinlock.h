/*
 * core/cortex/spinlock.h
 * Copyright (C) 2013 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef HALM_CORE_CORTEX_SPINLOCK_H_
#define HALM_CORE_CORTEX_SPINLOCK_H_
/*----------------------------------------------------------------------------*/
#include <stdbool.h>
#include <error.h>
/*----------------------------------------------------------------------------*/
void spinLock(spinlock_t *);
bool spinTryLock(spinlock_t *);
void spinUnlock(spinlock_t *);
/*----------------------------------------------------------------------------*/
#endif /* HALM_CORE_CORTEX_SPINLOCK_H_ */
