/*
 * core/cortex/m0/irq.h
 * Copyright (C) 2013 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef CORE_CORTEX_M0_IRQ_H_
#define CORE_CORTEX_M0_IRQ_H_
/*----------------------------------------------------------------------------*/
#include <core/core_defs.h>
#include <core/cortex/asm.h>
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
irqPriority irqGetPriority(irqNumber);
void irqSetPriority(irqNumber, irqPriority);
/*----------------------------------------------------------------------------*/
static inline void irqRestore(irqState state)
{
  __interruptsSetState(state);
}
/*----------------------------------------------------------------------------*/
static inline irqState irqSave(void)
{
  const irqState state = __interruptsGetState();

  __interruptsDisable();

  return state;
}
/*----------------------------------------------------------------------------*/
static inline void irqEnable(irqNumber irq)
{
  *NVIC->ISER = 1 << (irq & 0x1F);
}
/*----------------------------------------------------------------------------*/
static inline void irqDisable(irqNumber irq)
{
  *NVIC->ICER = 1 << (irq & 0x1F);
}
/*----------------------------------------------------------------------------*/
static inline void irqClearPending(irqNumber irq)
{
  *NVIC->ICPR = 1 << (irq & 0x1F);
}
/*----------------------------------------------------------------------------*/
static inline void irqSetPending(irqNumber irq)
{
  *NVIC->ISPR = 1 << (irq & 0x1F);
}
/*----------------------------------------------------------------------------*/
#endif /* CORE_CORTEX_M0_IRQ_H_ */
