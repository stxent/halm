/*
 * system.h
 * Copyright (C) 2013 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef SYSTEM_H_
#define SYSTEM_H_
/*----------------------------------------------------------------------------*/
#include <stdint.h>
/*----------------------------------------------------------------------------*/
extern uint32_t sysCoreClock;
/*----------------------------------------------------------------------------*/
static inline void usleep(uint32_t period)
{
  volatile uint32_t count = sysCoreClock / 3000000 * period;

  __asm__ __volatile__(
      "1:\n"
      "   SUBS.W %[count], %[count], #1\n"
      "   BNE 1b\n"
      : [count] "=r"(count)
      : "0" (count)
      : "r3"
  );
}
/*----------------------------------------------------------------------------*/
static inline void msleep(uint32_t period)
{
  usleep(1000 * period);
}
/*----------------------------------------------------------------------------*/
#endif /* SYSTEM_H_ */
