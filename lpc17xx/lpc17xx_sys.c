/*
 * lpc17xx_sys.c
 * Copyright (C) 2012 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <LPC17xx.h>
#include "lpc17xx_sys.h"
#include "macro.h"
/*----------------------------------------------------------------------------*/
/* Changes PCONP register, reset value 0x042887DE */
inline void sysPowerEnable(enum sysPowerDevice offset)
{
  LPC_SC->PCONP |= BIT(offset);
}
/*----------------------------------------------------------------------------*/
inline void sysPowerDisable(enum sysPowerDevice offset)
{
  LPC_SC->PCONP &= ~BIT(offset);
}
/*----------------------------------------------------------------------------*/
/* Exception for CAN1, CAN2 and CAN filtering: PCLK_DIV8 divides clock by 6 */
inline void sysClockControl(enum sysClockDevice offset, enum sysClockDiv value)
{
  uint32_t *ptr = (uint32_t *)(&LPC_SC->PCLKSEL0 + (offset >> 5));
  *ptr = (*ptr & ~(3 << (offset & 0x01F))) | (value << (offset & 0x01F));
}
/*----------------------------------------------------------------------------*/
