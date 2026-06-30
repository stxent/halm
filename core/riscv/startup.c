/*
 * startup.c
 * Copyright (C) 2026 xent
 * Project is distributed under the terms of the MIT License
 */

#include <xcore/core/riscv/csr.h>
#include <stddef.h>
/*----------------------------------------------------------------------------*/
void coreShutdown(void);
void coreStartup(void);
/*----------------------------------------------------------------------------*/
extern unsigned long _sdata;
extern unsigned long _edata;
extern unsigned long _sidata;
extern unsigned long _sbss;
extern unsigned long _ebss;
extern unsigned long _stack;

typedef void (*func_t)(void);
[[gnu::weak]] extern func_t __init_array_start;
[[gnu::weak]] extern func_t __init_array_end;
[[gnu::weak]] extern func_t __fini_array_start;
[[gnu::weak]] extern func_t __fini_array_end;
/*----------------------------------------------------------------------------*/
void coreShutdown(void)
{
  if (&__fini_array_start != NULL)
  {
    func_t *func;

    /* Call C/C++ destructors */
    for (func = &__fini_array_start; func < &__fini_array_end; func++)
      (*func)();
  }

  while (1);
}
/*----------------------------------------------------------------------------*/
void coreStartup(void)
{
  unsigned long *dst;
  unsigned long *src;

#ifdef CONFIG_CORE_RISCV_MEMORY_DEBUG
  /* Fill the RAM with a predefined pattern */
  for (dst = &_sdata; dst < &_stack;)
    *dst++ = CONFIG_CORE_CORTEX_MEMORY_PATTERN;
#endif

  /* Copy the data segment initializers from flash to RAM */
  for (dst = &_sdata, src = &_sidata; dst < &_edata;)
    *dst++ = *src++;

  /* Zero fill the BSS segment */
  for (dst = &_sbss; dst < &_ebss;)
    *dst++ = 0;

  if (&__init_array_start != NULL)
  {
    func_t *func;

    /* Call C/C++ constructors */
    for (func = &__init_array_start; func < &__init_array_end; func++)
      (*func)();
  }
}
