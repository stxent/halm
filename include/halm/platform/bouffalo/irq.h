/*
 *  halm/platform/bouffalo/irq.h
 * Copyright (C) 2026 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_CORE_RISCV_IRQ_H_
#error This header should not be included directly
#endif

#ifndef HALM_PLATFORM_BOUFFALO_IRQ_H_
#define HALM_PLATFORM_BOUFFALO_IRQ_H_
/*----------------------------------------------------------------------------*/
#include <halm/platform/bouffalo/clic_defs.h>
#include <halm/platform/bouffalo/csr_defs.h>
#include <halm/platform/platform_defs.h>
#include <xcore/asm.h>
#include <xcore/core/riscv/csr.h>
#include <xcore/helpers.h>
/*----------------------------------------------------------------------------*/
BEGIN_DECLS

static inline void irqRestore(IrqState state)
{
  csrSet(CSR_MSTATUS, state);
}

static inline IrqState irqSave(void)
{
  return csrReadAndClear(CSR_MSTATUS, MSTATUS_MIE) & MSTATUS_MIE;
}

static inline void irqEnable(IrqNumber irq)
{
  BL_CLIC->INTIE[irq] = CLICINTIE_ENABLE;
}

static inline void irqDisable(IrqNumber irq)
{
  BL_CLIC->INTIE[irq] = 0;
}

static inline void irqClearPending(IrqNumber irq)
{
  BL_CLIC->INTIP[irq] = 0;
}

static inline void irqSetPending(IrqNumber irq)
{
  BL_CLIC->INTIP[irq] = CLICINTIP_PENDING;
}

static inline bool irqStatus(IrqNumber irq)
{
  return BL_CLIC->INTIE[irq] != 0;
}

static inline IrqPriority irqGetPriority(IrqNumber irq)
{
  return CLICINTCFG_DIRECT_LEVEL_VALUE(BL_CLIC->INTCFG[irq]);
}

static inline void irqSetPriority(IrqNumber irq, IrqPriority priority)
{
  BL_CLIC->INTCFG[irq] = CLICINTCFG_DIRECT_LEVEL(priority);
}

static inline void irqSetLevelBits(unsigned int value)
{
  BL_CLIC->CLICCFG = (BL_CLIC->CLICCFG & ~CLICCFG_NLBITS_MASK)
      | CLICCFG_NLBITS(value);
}

static inline void irqVectoringEnable(void)
{
  BL_CLIC->CLICCFG |= CLICCFG_NVBITS;
}

static inline void irqVectoringDisable(void)
{
  BL_CLIC->CLICCFG &= ~CLICCFG_NVBITS;
}

END_DECLS
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_BOUFFALO_IRQ_H_ */
