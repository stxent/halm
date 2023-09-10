/*
 * halm/core/cortex/armv6m/irq.h
 * Copyright (C) 2013 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_CORE_CORTEX_IRQ_H_
#error This header should not be included directly
#endif

#ifndef HALM_CORE_CORTEX_ARMV6M_IRQ_H_
#define HALM_CORE_CORTEX_ARMV6M_IRQ_H_
/*----------------------------------------------------------------------------*/
#include <halm/core/core_defs.h>
#include <xcore/asm.h>
#include <stdbool.h>
/*----------------------------------------------------------------------------*/
enum
{
  /* Core-specific interrupt sources */
  NMI_IRQ       = -14,
  HARDFAULT_IRQ = -12,
  SVCALL_IRQ    = -5,
  PENDSV_IRQ    = -2,
  SYSTICK_IRQ   = -1,

  /* Undefined interrupt source */
  IRQ_RESERVED  = -127
};
/*----------------------------------------------------------------------------*/
BEGIN_DECLS

IrqPriority irqGetPriority(IrqNumber);
void irqSetPriority(IrqNumber, IrqPriority);

END_DECLS
/*----------------------------------------------------------------------------*/
BEGIN_DECLS

static inline void irqRestore(IrqState state)
{
  __msr_primask(state);
}

static inline IrqState irqSave(void)
{
  const IrqState state = __mrs_primask();
  __cpsid();
  return state;
}

static inline void irqEnable(IrqNumber irq)
{
  NVIC->ISER = 1UL << (irq & 0x1F);
}

static inline void irqDisable(IrqNumber irq)
{
  NVIC->ICER = 1UL << (irq & 0x1F);
  __dsb();
  __isb();
}

static inline void irqClearPending(IrqNumber irq)
{
  NVIC->ICPR = 1UL << (irq & 0x1F);
}

static inline void irqSetPending(IrqNumber irq)
{
  NVIC->ISPR = 1UL << (irq & 0x1F);
}

static inline bool irqStatus(IrqNumber irq)
{
  return (NVIC->ISER & (1UL << (irq & 0x1F))) != 0;
}

END_DECLS
/*----------------------------------------------------------------------------*/
#endif /* HALM_CORE_CORTEX_ARMV6M_IRQ_H_ */
