/*
 * halm/core/cortex/armv7m/irq.h
 * Copyright (C) 2014 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_CORE_CORTEX_IRQ_H_
#error This header should not be included directly
#endif

#ifndef HALM_CORE_CORTEX_ARMV7M_IRQ_H_
#define HALM_CORE_CORTEX_ARMV7M_IRQ_H_
/*----------------------------------------------------------------------------*/
#include <halm/core/core_defs.h>
#include <xcore/asm.h>
#include <stdbool.h>
/*----------------------------------------------------------------------------*/
enum
{
  /* Core-specific interrupt sources */
  NMI_IRQ         = -14,
  MEMMANAGE_IRQ   = -12,
  BUSFAULT_IRQ    = -11,
  USAGEFAULT_IRQ  = -10,
  SVCALL_IRQ      = -5,
  DEBUGMON_IRQ    = -4,
  PENDSV_IRQ      = -2,
  SYSTICK_IRQ     = -1,

  /* Undefined interrupt source */
  IRQ_RESERVED    = -127
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
#ifdef CONFIG_IRQ_THRESHOLD
  __interruptsResetBasePriority(state);
#else
  __interruptsSetState(state);
#endif
}

static inline IrqState irqSave(void)
{
  IrqState state;

#ifdef CONFIG_IRQ_THRESHOLD
  state = __interruptsGetBasePriority();
  __interruptsSetBasePriority(IRQ_PRIORITY_TO_REG(CONFIG_IRQ_THRESHOLD));
#else
  state = __interruptsGetState();
  __interruptsDisable();
#endif

  return state;
}

static inline void irqEnable(IrqNumber irq)
{
  NVIC->ISER[irq >> 5] = 1UL << (irq & 0x1F);
}

static inline void irqDisable(IrqNumber irq)
{
  NVIC->ICER[irq >> 5] = 1UL << (irq & 0x1F);
  __dsb();
  __isb();
}

static inline void irqClearPending(IrqNumber irq)
{
  NVIC->ICPR[irq >> 5] = 1UL << (irq & 0x1F);
}

static inline void irqSetPending(IrqNumber irq)
{
  NVIC->ISPR[irq >> 5] = 1UL << (irq & 0x1F);
}

static inline bool irqStatus(IrqNumber irq)
{
  return (NVIC->ISER[irq >> 5] & (1UL << (irq & 0x1F))) != 0;
}

END_DECLS
/*----------------------------------------------------------------------------*/
#endif /* HALM_CORE_CORTEX_ARMV7M_IRQ_H_ */
