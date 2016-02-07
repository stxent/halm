/*
 * system.c
 * Copyright (C) 2014 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <platform/nxp/lpc11xx/system.h>
#include <platform/nxp/lpc11xx/system_defs.h>
#include <platform/platform_defs.h>
/*----------------------------------------------------------------------------*/
uint8_t sysFlashLatency()
{
  return FLASHCFG_FLASHTIM_VALUE(LPC_FMC->FLASHCFG) + 1;
}
/*----------------------------------------------------------------------------*/
/**
 * Set the flash access time.
 * @param value Flash access time in CPU clocks.
 * @n Possible values and recommended operating frequencies:
 *   - 1 clock: up to 20 MHz.
 *   - 2 clocks: up to 40 MHz.
 *   - 3 clocks: up to 50 MHz.
 */
void sysFlashLatencyUpdate(uint8_t value)
{
  LPC_FMC->FLASHCFG = (LPC_FMC->FLASHCFG & ~FLASHCFG_FLASHTIM_MASK)
      | FLASHCFG_FLASHTIM(value - 1);
}
