/*
 * power.c
 * Copyright (C) 2012 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <macro.h>
#include <platform/nxp/device_defs.h>
#include <platform/nxp/lpc17xx/power.h>
/*----------------------------------------------------------------------------*/
/* Changes PCONP register, reset value 0x042887DE */
inline void sysPowerEnable(enum sysPowerDevice block)
{
  LPC_SC->PCONP |= BIT(block);
}
/*----------------------------------------------------------------------------*/
inline void sysPowerDisable(enum sysPowerDevice block)
{
  LPC_SC->PCONP &= ~BIT(block);
}
/*----------------------------------------------------------------------------*/
/* Exception for CAN1, CAN2 and CAN filtering: PCLK_DIV8 divides clock by 6 */
inline void sysClockControl(enum sysClockDevice block, enum sysClockDiv value)
{
  uint32_t *ptr = (uint32_t *)(&LPC_SC->PCLKSEL0 + (block >> 5));
  *ptr = (*ptr & ~(3 << (block & 0x01F))) | (value << (block & 0x01F));
}
