/*
 * halm/core/cortex/m0/delay.h
 * Copyright (C) 2013 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef HALM_CORE_CORTEX_M0_DELAY_H_
#define HALM_CORE_CORTEX_M0_DELAY_H_
/*----------------------------------------------------------------------------*/
#include <stdint.h>
#include <xcore/helpers.h>
/*----------------------------------------------------------------------------*/
BEGIN_DECLS

static inline void __delay(uint32_t count)
{
  __asm__ volatile (
      ".syntax unified\n"
      "1:\n"
      "    NOP\n"
      "    SUBS %[count], %[count], #1\n"
      "    BCS 1b\n"
      ".syntax divided"
      : [count] "=r" (count)
      : "0" (count)
      : "r3"
  );
}

static inline void delayTicks(uint32_t count)
{
  __delay(count >> 2);
}

static inline void mdelay(uint32_t period)
{
  extern uint32_t ticksPerSecond;

  while (period)
  {
    uint32_t count = period > (1 << 12) ? (1 << 12) : period;

    period -= count;
    count = (ticksPerSecond * count) >> 2;
    __delay(count);
  }
}

static inline void udelay(uint32_t period)
{
  extern uint32_t ticksPerSecond;

  while (period)
  {
    uint32_t count = period > (1 << 12) ? (1 << 12) : period;

    period -= count;
    count = (ticksPerSecond * count) / 4000;
    __delay(count);
  }
}

END_DECLS
/*----------------------------------------------------------------------------*/
#endif /* HALM_CORE_CORTEX_M0_DELAY_H_ */
