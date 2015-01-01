/*
 * core/cortex/m4/irq.h
 * Copyright (C) 2014 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef CORE_CORTEX_M4_IRQ_H_
#define CORE_CORTEX_M4_IRQ_H_
/*----------------------------------------------------------------------------*/
#include <core/cortex/core_defs.h>
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
priority_t irqGetPriority(irq_t);
void irqSetPriority(irq_t, priority_t);
/*----------------------------------------------------------------------------*/
static inline void interruptsDisable(void)
{
  __interruptsDisable();
}
/*----------------------------------------------------------------------------*/
static inline void interruptsEnable(void)
{
  __interruptsEnable();
}
/*----------------------------------------------------------------------------*/
static inline void irqEnable(irq_t irq)
{
  *(NVIC->ISER + (irq >> 5)) = 1 << (irq & 0x1F);
}
/*----------------------------------------------------------------------------*/
static inline void irqDisable(irq_t irq)
{
  *(NVIC->ICER + (irq >> 5)) = 1 << (irq & 0x1F);
}
/*----------------------------------------------------------------------------*/
static inline void irqClearPending(irq_t irq)
{
  *(NVIC->ICPR + (irq >> 5)) = 1 << (irq & 0x1F);
}
/*----------------------------------------------------------------------------*/
static inline void irqSetPending(irq_t irq)
{
  *(NVIC->ISPR + (irq >> 5)) = 1 << (irq & 0x1F);
}
/*----------------------------------------------------------------------------*/
#endif /* CORE_CORTEX_M4_IRQ_H_ */
