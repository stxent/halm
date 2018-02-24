/*
 * halm/core/x86/irq.h
 * Copyright (C) 2018 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef HALM_CORE_X86_IRQ_H_
#define HALM_CORE_X86_IRQ_H_
/*----------------------------------------------------------------------------*/
typedef unsigned int IrqState;
/*----------------------------------------------------------------------------*/
static inline void irqRestore(IrqState state __attribute__((unused)))
{
}

static inline IrqState irqSave(void)
{
  return 0;
}
/*----------------------------------------------------------------------------*/
#endif /* HALM_CORE_X86_IRQ_H_ */
