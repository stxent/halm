/*
 * pm.h
 * Copyright (C) 2014 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

/**
 * @file
 * Power Management interface for embedded systems.
 */

#ifndef PM_H_
#define PM_H_
/*----------------------------------------------------------------------------*/
#include <stdbool.h>
#include <stdint.h>
/*----------------------------------------------------------------------------*/
enum pmState
{
  PM_ACTIVE = 0,
  PM_SLEEP,
  PM_SUSPEND,
  PM_POWERDOWN
};
/*----------------------------------------------------------------------------*/
typedef enum result (*PmCallback)(void *, enum pmState);
/*----------------------------------------------------------------------------*/
enum result pmChangeState(enum pmState);
enum result pmRegister(void *, PmCallback);
void pmUnregister(const void *);
/*----------------------------------------------------------------------------*/
#endif /* PM_H_ */
