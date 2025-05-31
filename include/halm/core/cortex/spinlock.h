/*
 * halm/core/cortex/spinlock.h
 * Copyright (C) 2013 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_SPINLOCK_H_
#error This header should not be included directly
#endif

#ifndef HALM_CORE_CORTEX_SPINLOCK_H_
#define HALM_CORE_CORTEX_SPINLOCK_H_
/*----------------------------------------------------------------------------*/
#include <xcore/error.h>
#include <xcore/helpers.h>
/*----------------------------------------------------------------------------*/
BEGIN_DECLS

void spinLock(Spinlock *);
bool spinTryLock(Spinlock *);
void spinUnlock(Spinlock *);

END_DECLS
/*----------------------------------------------------------------------------*/
#endif /* HALM_CORE_CORTEX_SPINLOCK_H_ */
