/*
 * halm/core/riscv/delay.h
 * Copyright (C) 2026 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_DELAY_H_
#error This header should not be included directly
#endif

#ifndef HALM_CORE_RISCV_DELAY_H_
#define HALM_CORE_RISCV_DELAY_H_
/*----------------------------------------------------------------------------*/
#include <xcore/helpers.h>
#include <stdint.h>
/*----------------------------------------------------------------------------*/
BEGIN_DECLS

static inline void mdelay(uint32_t period)
{
  // TODO
  for (volatile unsigned i = 0; i < period * 10000; ++i);
}

static inline void udelay(uint32_t period)
{
  // TODO
  for (volatile unsigned i = 0; i < period * 10; ++i);
}

END_DECLS
/*----------------------------------------------------------------------------*/
#endif /* HALM_CORE_RISCV_DELAY_H_ */
