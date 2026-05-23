/*
 * startup.c
 * Copyright (C) 2026 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/irq.h>
#include <halm/platform/bouffalo/bl602/l1c.h>
#include <halm/platform/bouffalo/clic_defs.h>
#include <halm/platform/bouffalo/csr_defs.h>
#include <xcore/core/riscv/csr.h>
#include <stddef.h>
/*----------------------------------------------------------------------------*/
/* #define MODE_CLINT */
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
  for (IrqNumber index = 0; index < IRQ_END; ++index)
  {
    irqDisable(index);
    irqSetPriority(index, 0);
  }
  irqSetLevelBits(0);
  irqVectoringDisable();

  csrWrite(CSR_MTVEC, (uintptr_t)commonIrqHandler
      | MTVEC_MODE(MODE_CLIC_VECTORED));
  csrWrite(CSR_MTVT, (uintptr_t)vector_table);
#endif

  /* Configure L1 cache controller */
#ifdef CONFIG_PLATFORM_BOUFFALO_ICACHE_8K
  cacheEnable(ICACHE_8K);
#elifdef CONFIG_PLATFORM_BOUFFALO_ICACHE_16K
  cacheEnable(ICACHE_16K);
#elifdef CONFIG_PLATFORM_BOUFFALO_ICACHE_24K
  cacheEnable(ICACHE_24K);
#elifdef CONFIG_PLATFORM_BOUFFALO_ICACHE_32K
  cacheEnable(ICACHE_32K);
#else
  cacheDisable();
#endif

  /* Enable global interrupts */
  csrSet(CSR_MSTATUS, MSTATUS_MIE);
}
