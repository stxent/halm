/*
 * core/cortex/m4/delay.h
 * Copyright (C) 2013 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef CORE_CORTEX_M4_DELAY_H_
#define CORE_CORTEX_M4_DELAY_H_
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
  //FIXME Recalculate delay
  volatile uint32_t count = coreClock / 3000000 * period;

  __asm__ volatile (
      "1:\n\t"
      "   SUBS.W %[count], %[count], #1\n\t"
      "   BNE 1b"
      : [count] "=r" (count)
      : "0" (count)
      : "r3"
  );
}
/*----------------------------------------------------------------------------*/
#endif /* CORE_CORTEX_M4_DELAY_H_ */
