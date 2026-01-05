/*
 * halm/platform/bouffalo/delay.h
 * Copyright (C) 2026 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_CORE_RISCV_DELAY_H_
#error This header should not be included directly
#endif

#ifndef HALM_PLATFORM_BOUFFALO_DELAY_H_
#define HALM_PLATFORM_BOUFFALO_DELAY_H_
/*----------------------------------------------------------------------------*/
#include <halm/platform/bouffalo/csr_defs.h>
#include <xcore/core/riscv/csr.h>
/*----------------------------------------------------------------------------*/
BEGIN_DECLS

static inline void __delay(uint32_t count)
{
  const uint32_t start = csrRead(CSR_MCYCLE);
  while (csrRead(CSR_MCYCLE) - start < count);
}

static inline void delayTicks(uint32_t count)
{
  __delay(count);
}

static inline void mdelay(uint32_t period)
{
  extern uint32_t ticksPerSecond;
  const uint32_t cyclesPerQuanta = ticksPerSecond * 1000 >> 10;

  while (period)
  {
    uint32_t count = MIN(period, 1000);

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
    uint32_t count = MIN(period, 1000);

    period -= count;
    count = cyclesPerQuanta * count >> 10;
    __delay(count);
  }
}

END_DECLS
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_BOUFFALO_DELAY_H_ */
