/*
 * halm/core/x86/irq.h
 * Copyright (C) 2018 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_IRQ_H_
#error This header should not be included directly
#endif

#ifndef HALM_CORE_X86_IRQ_H_
#define HALM_CORE_X86_IRQ_H_
/*----------------------------------------------------------------------------*/
#include <xcore/helpers.h>
#include <stdbool.h>
/*----------------------------------------------------------------------------*/
typedef unsigned int IrqNumber;
typedef unsigned int IrqPriority;
typedef unsigned int IrqState;
/*----------------------------------------------------------------------------*/
BEGIN_DECLS

static inline void irqRestore(IrqState)
{
}

static inline IrqState irqSave(void)
{
  return 0;
}

static inline IrqPriority irqGetPriority(IrqNumber)
{
  return 0;
}

static inline void irqSetPriority(IrqNumber, IrqPriority)
{
}

static inline void irqEnable(IrqNumber)
{
}

static inline void irqDisable(IrqNumber)
{
}

static inline void irqClearPending(IrqNumber)
{
}

static inline void irqSetPending(IrqNumber)
{
}

static inline bool irqStatus(IrqNumber)
{
  return false;
}

END_DECLS
/*----------------------------------------------------------------------------*/
#endif /* HALM_CORE_X86_IRQ_H_ */
