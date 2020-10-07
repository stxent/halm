/*
 * halm/spinlock.h
 * Copyright (C) 2013 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef HALM_SPINLOCK_H_
#define HALM_SPINLOCK_H_
/*----------------------------------------------------------------------------*/
#include <halm/target.h>
#include <stdint.h>
/*----------------------------------------------------------------------------*/
typedef volatile uint8_t Spinlock;

enum
{
  SPIN_UNLOCKED,
  SPIN_LOCKED
};
/*----------------------------------------------------------------------------*/
#undef HEADER_PATH
#define HEADER_PATH <halm/core/CORE_TYPE/spinlock.h>
#include HEADER_PATH
#undef HEADER_PATH
/*----------------------------------------------------------------------------*/
#endif /* HALM_SPINLOCK_H_ */
