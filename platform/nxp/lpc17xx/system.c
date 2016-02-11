/*
 * system.c
 * Copyright (C) 2012 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <stdbool.h>
#include <platform/nxp/lpc17xx/clocking_defs.h>
#include <platform/nxp/lpc17xx/system.h>
#include <platform/nxp/lpc17xx/system_defs.h>
/*----------------------------------------------------------------------------*/
/**
 * Set the peripheral clock prescaler.
 * @param peripheral Peripheral to be configured.
 * @param divisor Clock divisor.
 * @b CLK_DIV6 is only used for CAN1, CAN2 and CAN Filtering peripherals.
 * @b CLK_DIV8 is unavailable for CAN peripherals.
 */
void sysClockControl(enum sysClockDevice peripheral, enum sysClockDiv divisor)
{
  unsigned int value;

  switch (divisor)
  {
    case CLK_DIV1:
    case CLK_DIV2:
      value = divisor + 1;
      break;

    case CLK_DIV4:
      value = 0;
      break;

    default:
      value = 3;
      break;
  }

  volatile uint32_t * const reg = &LPC_SC->PCLKSEL0 + (peripheral >> 5);
  const unsigned int offset = peripheral & 0x1F;
  const bool pllEnabled = (LPC_SC->PLL0STAT & PLL0STAT_CONNECTED) != 0;

  /* PCLKSEL and PLL0 workaround */
  if (pllEnabled)
  {
    LPC_SC->PLL0CON &= ~PLL0CON_CONNECT;
    LPC_SC->PLL0FEED = PLLFEED_FIRST;
    LPC_SC->PLL0FEED = PLLFEED_SECOND;
  }

  *reg = (*reg & ~(3 << offset)) | (value << offset);

  if (pllEnabled)
  {
    LPC_SC->PLL0CON |= PLL0CON_CONNECT;
    LPC_SC->PLL0FEED = PLLFEED_FIRST;
    LPC_SC->PLL0FEED = PLLFEED_SECOND;
  }
}
/*----------------------------------------------------------------------------*/
/**
 * Set the flash access time.
 * @param value Flash access time in CPU clocks.
 * @n Possible values and recommended operating frequencies:
 *   - 1 clock: up to 20 MHz.
 *   - 2 clocks: up to 40 MHz.
 *   - 3 clocks: up to 60 MHz.
 *   - 4 clocks: up to 80 MHz.
 *   - 5 clocks: up to 100 MHz or 120 MHz on some parts.
 *   - 6 clocks: safe setting will work under any conditions.
 */
void sysFlashLatency(unsigned int value)
{
  LPC_SC->FLASHCFG = (LPC_SC->FLASHCFG & ~FLASHCFG_FLASHTIM_MASK)
      | FLASHCFG_FLASHTIM(value - 1);
}
