/*
 * spinlock.h
 * Copyright (C) 2013 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef SPINLOCK_TOP_H_
#define SPINLOCK_TOP_H_
/*----------------------------------------------------------------------------*/
#include <stdint.h>
#include <mcu.h>
/*----------------------------------------------------------------------------*/
enum
{
  SPIN_UNLOCKED = 0,
  SPIN_LOCKED
};
/*----------------------------------------------------------------------------*/
typedef volatile uint8_t spinlock_t;
/*----------------------------------------------------------------------------*/
#define HEADER_PATH <core/CORE_TYPE/spinlock.h>
#include HEADER_PATH
#undef HEADER_PATH
/*----------------------------------------------------------------------------*/
#endif /* SPINLOCK_TOP_H_ */
