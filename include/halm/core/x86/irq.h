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

static inline void irqRestore([[maybe_unused]] IrqState state)
{
}

static inline IrqState irqSave(void)
{
  return 0;
}

static inline IrqPriority irqGetPriority([[maybe_unused]] IrqNumber irq)
{
  return 0;
}

static inline void irqSetPriority([[maybe_unused]] IrqNumber irq,
    [[maybe_unused]] IrqPriority priority)
{
}

static inline void irqEnable([[maybe_unused]] IrqNumber irq)
{
}

static inline void irqDisable([[maybe_unused]] IrqNumber irq)
{
}

static inline void irqClearPending([[maybe_unused]] IrqNumber irq)
{
}

static inline void irqSetPending([[maybe_unused]] IrqNumber irq)
{
}

static inline bool irqStatus([[maybe_unused]] IrqNumber irq)
{
  return false;
}

END_DECLS
/*----------------------------------------------------------------------------*/
#endif /* HALM_CORE_X86_IRQ_H_ */
