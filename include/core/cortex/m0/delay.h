/*
 * core/cortex/m0/delay.h
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
  extern uint32_t coreClock;
  volatile uint32_t count = coreClock / 3000000 * period;

  __asm__ volatile (
      ".syntax unified\n\t"
      "1:\n\t"
      "   SUBS %[count], %[count], #1\n\t"
      "   BNE 1b\n\t"
      ".syntax divided"
      : [count] "=r" (count)
      : "0" (count)
      : "r3"
  );
}
/*----------------------------------------------------------------------------*/
#endif /* DELAY_H_ */
