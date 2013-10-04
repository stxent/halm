/*
 * clock.h
 * Copyright (C) 2013 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef CLOCK_H_
#define CLOCK_H_
/*----------------------------------------------------------------------------*/
#include <stdint.h>
#include <stdbool.h>
#include "entity.h"
/*----------------------------------------------------------------------------*/
/* Class descriptor */
struct ClockClass
{
  CLASS_HEADER

  void (*disable)(void);
  enum result (*enable)(const void *);
  bool (*ready)(void);
};
/*----------------------------------------------------------------------------*/
void clockDisable(const void *);
enum result clockEnable(const void *, const void *);
bool clockReady(const void *);
/*----------------------------------------------------------------------------*/
#endif /* CLOCK_H_ */
