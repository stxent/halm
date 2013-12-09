/*
 * system.c
 * Copyright (C) 2012 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <platform/nxp/lpc17xx/system.h>
#include <platform/nxp/lpc17xx/system_defs.h>
/*----------------------------------------------------------------------------*/
/* Exception for CAN1, CAN2 and CAN filtering: PCLK_DIV8 divides clock by 6 */
void sysClockControl(enum sysClockDevice block, enum sysClockDiv value)
{
  uint32_t *ptr = (uint32_t *)(&LPC_SC->PCLKSEL0 + (block >> 5));
  *ptr = (*ptr & ~(3 << (block & 0x01F))) | (value << (block & 0x01F));
}
/*----------------------------------------------------------------------------*/
void sysFlashLatency(uint8_t value)
{
  LPC_SC->FLASHCFG = (LPC_SC->FLASHCFG & ~FLASHCFG_FLASHTIM_MASK)
      | FLASHCFG_FLASHTIM(value);
}
