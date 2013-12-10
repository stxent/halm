/*
 * system.c
 * Copyright (C) 2012 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <platform/nxp/lpc17xx/system.h>
#include <platform/nxp/lpc17xx/system_defs.h>
/*----------------------------------------------------------------------------*/
/**
 * Set clock prescaler for specified peripheral.
 * @param peripheral Peripheral to be configured.
 * @param value Prescaler value.
 * @b CLK_DIV6 only used for CAN1, CAN2 and CAN Filtering peripherals.
 * @b CLK_DIV8 is unavailable for CAN peripherals.
 */
void sysClockControl(enum sysClockDevice peripheral, enum sysClockDiv value)
{
  uint32_t *reg = (uint32_t *)(&LPC_SC->PCLKSEL0 + (peripheral >> 5));

  *reg = (*reg & ~(3 << (peripheral & 0x01F)))
      | (value << (peripheral & 0x01F));
}
/*----------------------------------------------------------------------------*/
/**
 * Set flash access time.
 * @param value Flash access time in CPU clocks.
 * @n Possible values and recommended operating frequencies:
 *   - 1 clock: up to 20 MHz.
 *   - 2 clocks: up to 40 MHz.
 *   - 3 clocks: up to 60 MHz.
 *   - 4 clocks: up to 80 MHz.
 *   - 5 clocks: up to 100 MHz or 120 MHz on some parts.
 *   - 6 clocks: safe setting will work under any conditions.
 */
void sysFlashLatency(uint8_t value)
{
  LPC_SC->FLASHCFG = (LPC_SC->FLASHCFG & ~FLASHCFG_FLASHTIM_MASK)
      | FLASHCFG_FLASHTIM(value - 1);
}
