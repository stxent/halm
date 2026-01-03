/*
 * startup.c
 * Copyright (C) 2026 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/irq.h>
#include <halm/platform/bouffalo/clic_defs.h>
#include <halm/platform/bouffalo/csr_defs.h>
#include <xcore/core/riscv/csr.h>
#include <stddef.h>
/*----------------------------------------------------------------------------*/
/*  #define MODE_CLINT */
/*----------------------------------------------------------------------------*/
extern void (* const vector_table[])(void);
extern void trap_trampoline(void);
/*----------------------------------------------------------------------------*/
[[gnu::interrupt]] [[gnu::section(".vectors.trap")]] void commonIrqHandler(void)
{
  const uint32_t mcause = csrRead(CSR_MCAUSE);
  (void)mcause;

#ifdef MODE_CLINT
  /* [[gnu::interrupt]] attribute must be removed */
  if (mcause & MCAUSE_INTERRUPT)
  {
    const uint32_t number = MCAUSE_EXCCODE_VALUE(mcause);
    vector_table[number]();
  }
#endif
}
/*----------------------------------------------------------------------------*/
void platformStartup(void)
{
  csrClear(CSR_MSTATUS, MSTATUS_MIE);
  csrWrite(CSR_MIE, 0);

#ifdef MODE_CLINT
  csrWrite(CSR_MTVEC, (uintptr_t)trap_trampoline
      | MTVEC_MODE(MODE_CLINT_DIRECT));
  csrSet(CSR_MIE, MIE_MEIE);
#else
  for (size_t index = 0; index < IRQ_END; ++index)
  {
    irqDisable((IrqNumber)index);
    irqSetPriority((IrqNumber)index, 0);
  }
  irqSetLevelBits(0);
  irqVectoringDisable();

  csrWrite(CSR_MTVEC, (uintptr_t)commonIrqHandler
      | MTVEC_MODE(MODE_CLIC_VECTORED));
  csrWrite(CSR_MTVT, (uintptr_t)vector_table);
#endif

  /* Enable global interrupts */
  csrSet(CSR_MSTATUS, MSTATUS_MIE);
}
