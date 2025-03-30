/*
 * halm/core/cortex/armv6m/delay.h
 * Copyright (C) 2013 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_CORE_CORTEX_DELAY_H_
#error This header should not be included directly
#endif

#ifndef HALM_CORE_CORTEX_ARMV6M_DELAY_H_
#define HALM_CORE_CORTEX_ARMV6M_DELAY_H_
/*----------------------------------------------------------------------------*/
#include <xcore/helpers.h>
#include <stdint.h>
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
  );
}

static inline void delayTicks(uint32_t count)
{
  __delay(count >> 2);
}

static inline void mdelay(uint32_t period)
{
  extern uint32_t ticksPerSecond;
  const uint32_t cyclesPerQuanta = ticksPerSecond * 1000 >> 12;

  while (period)
  {
    uint32_t count = MIN(period, 4000);

    period -= count;
    count = cyclesPerQuanta * count;
    __delay(count);
  }
}

static inline void udelay(uint32_t period)
{
  extern uint32_t ticksPerSecond;
  const uint32_t cyclesPerQuanta = ticksPerSecond;

  while (period)
  {
    uint32_t count = MIN(period, 4000);

    period -= count;
    count = cyclesPerQuanta * count >> 12;
    __delay(count);
  }
}

END_DECLS
/*----------------------------------------------------------------------------*/
#endif /* HALM_CORE_CORTEX_ARMV6M_DELAY_H_ */
