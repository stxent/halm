/*
 * system.c
 * Copyright (C) 2014 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/platform/lpc/system.h>
#include <halm/platform/lpc/system_defs.h>
/*----------------------------------------------------------------------------*/
unsigned int sysFlashLatency(void)
{
  return FLASHCFG_FLASHTIM_VALUE(LPC_FMC->FLASHCFG) + 1;
}
/*----------------------------------------------------------------------------*/
unsigned int sysFlashLatencyFromFrequency(uint32_t frequency)
{
  if (frequency <= 20000000)
    return 1;
  if (frequency <= 40000000)
    return 2;
  return 3;
}
/*----------------------------------------------------------------------------*/
/**
 * Set the flash access time.
 *
 * Possible values and recommended operating frequencies:
 * - 1 clock cycle: up to 20 MHz.
 * - 2 clock cycles: up to 40 MHz.
 * - 3 clock cycles: up to 50 MHz.
 *
 * @param value Flash access time in CPU clocks.
 */
void sysFlashLatencyUpdate(unsigned int value)
{
  LPC_FMC->FLASHCFG = (LPC_FMC->FLASHCFG & ~FLASHCFG_FLASHTIM_MASK)
      | FLASHCFG_FLASHTIM(value - 1);
}
/*----------------------------------------------------------------------------*/
void sysFlashLatencyReset(void)
{
  /* Set safe latency settings */
  sysFlashLatencyUpdate(3);
}
/*----------------------------------------------------------------------------*/
void sysMemoryRemap(enum MemoryRemap value)
{
  LPC_SYSCON->SYSMEMREMAP = SYSMEMREMAP_MAP(value);
}
