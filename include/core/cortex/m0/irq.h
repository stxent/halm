/*
 * core/cortex/m0/irq.h
 * Copyright (C) 2013 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef IRQ_H_
#define IRQ_H_
/*----------------------------------------------------------------------------*/
#include <platform/nxp/platform_defs.h> //FIXME
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
static inline void irqEnable(irq_t irq)
{
  *NVIC->ISER = 1 << (irq & 0x1F);
}
/*----------------------------------------------------------------------------*/
static inline void irqDisable(irq_t irq)
{
  *NVIC->ICER = 1 << (irq & 0x1F);
}
/*----------------------------------------------------------------------------*/
static inline void irqClearPending(irq_t irq)
{
  *NVIC->ICPR = 1 << (irq & 0x1F);
}
/*----------------------------------------------------------------------------*/
static inline void irqSetPending(irq_t irq)
{
  *NVIC->ISPR = 1 << (irq & 0x1F);
}
/*----------------------------------------------------------------------------*/
#endif /* IRQ_H_ */
