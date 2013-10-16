/*
 * core/cortex/m3/delay.h
 * Copyright (C) 2013 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef DELAY_H_
#define DELAY_H_
/*----------------------------------------------------------------------------*/
#include <stdint.h>
/*----------------------------------------------------------------------------*/
static inline void mdelay(uint32_t);
static inline void udelay(uint32_t);
/*----------------------------------------------------------------------------*/
static inline void mdelay(uint32_t period)
{
  udelay(1000 * period);
}
/*----------------------------------------------------------------------------*/
static inline void udelay(uint32_t period)
{
//  volatile uint32_t count = sysCoreClock / 3000000 * period;
  volatile uint32_t count = 12000000 / 3000000 * period;

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
#endif /* DELAY_H_ */
