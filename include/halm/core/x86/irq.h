/*
 * halm/core/x86/irq.h
 * Copyright (C) 2018 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef HALM_CORE_X86_IRQ_H_
#define HALM_CORE_X86_IRQ_H_
/*----------------------------------------------------------------------------*/
#include <stdbool.h>
#include <xcore/helpers.h>
/*----------------------------------------------------------------------------*/
typedef unsigned int IrqNumber;
typedef unsigned int IrqPriority;
typedef unsigned int IrqState;
/*----------------------------------------------------------------------------*/
BEGIN_DECLS

static inline void irqRestore(IrqState state __attribute__((unused)))
{
}

static inline IrqState irqSave(void)
{
  return 0;
}

static inline IrqPriority irqGetPriority(IrqNumber irq __attribute__((unused)))
{
  return 0;
}

static inline void irqSetPriority(IrqNumber irq __attribute__((unused)),
    IrqPriority priority __attribute__((unused)))
{
}

static inline void irqEnable(IrqNumber irq __attribute__((unused)))
{
}

static inline void irqDisable(IrqNumber irq __attribute__((unused)))
{
}

static inline void irqClearPending(IrqNumber irq __attribute__((unused)))
{
}

static inline void irqSetPending(IrqNumber irq __attribute__((unused)))
{
}

static inline bool irqStatus(IrqNumber irq __attribute__((unused)))
{
  return false;
}

END_DECLS
/*----------------------------------------------------------------------------*/
#endif /* HALM_CORE_X86_IRQ_H_ */
