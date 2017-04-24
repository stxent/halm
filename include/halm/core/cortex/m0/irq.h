/*
 * halm/core/cortex/m0/irq.h
 * Copyright (C) 2013 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef HALM_CORE_CORTEX_M0_IRQ_H_
#define HALM_CORE_CORTEX_M0_IRQ_H_
/*----------------------------------------------------------------------------*/
#include <xcore/core/cortex/asm.h>
#include <halm/core/core_defs.h>
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
  SYSTICK_IRQ     = -1
};
/*----------------------------------------------------------------------------*/
IrqPriority irqGetPriority(IrqNumber);
void irqSetPriority(IrqNumber, IrqPriority);
/*----------------------------------------------------------------------------*/
static inline void irqRestore(IrqState state)
{
  __interruptsSetState(state);
}
/*----------------------------------------------------------------------------*/
static inline IrqState irqSave(void)
{
  const IrqState state = __interruptsGetState();

  __interruptsDisable();

  return state;
}
/*----------------------------------------------------------------------------*/
static inline void irqEnable(IrqNumber irq)
{
  *NVIC->ISER = 1UL << (irq & 0x1F);
}
/*----------------------------------------------------------------------------*/
static inline void irqDisable(IrqNumber irq)
{
  *NVIC->ICER = 1UL << (irq & 0x1F);
}
/*----------------------------------------------------------------------------*/
static inline void irqClearPending(IrqNumber irq)
{
  *NVIC->ICPR = 1UL << (irq & 0x1F);
}
/*----------------------------------------------------------------------------*/
static inline void irqSetPending(IrqNumber irq)
{
  *NVIC->ISPR = 1UL << (irq & 0x1F);
}
/*----------------------------------------------------------------------------*/
#endif /* HALM_CORE_CORTEX_M0_IRQ_H_ */
