/*
 * startup.c
 * Copyright (C) 2026 xent
 * Project is distributed under the terms of the MIT License
 */

#include <xcore/core/riscv/csr.h>
/*----------------------------------------------------------------------------*/
extern unsigned long _sdata;
extern unsigned long _edata;
extern unsigned long _sidata;
extern unsigned long _sbss;
extern unsigned long _ebss;
extern unsigned long _stack;
/*----------------------------------------------------------------------------*/
void coreStartup(void)
{
  register unsigned long *dst;
  register unsigned long *src;

#ifdef CONFIG_CORE_RISCV_MEMORY_DEBUG
  /* Fill the RAM with a predefined pattern */
  for (dst = &_sdata; dst < &_stack;)
    *dst++ = CONFIG_CORE_CORTEX_MEMORY_PATTERN;
#endif

  /* Copy the data segment initializers from Flash to RAM */
  for (dst = &_sdata, src = &_sidata; dst < &_edata;)
    *dst++ = *src++;

  /* Zero fill the BSS segment */
  for (dst = &_sbss; dst < &_ebss;)
    *dst++ = 0;
}
