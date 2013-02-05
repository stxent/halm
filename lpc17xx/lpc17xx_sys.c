/*
 * lpc17xx_sys.c
 * Copyright (C) 2012 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include "LPC17xx.h"
/*----------------------------------------------------------------------------*/
#include "lpc17xx_sys.h"
#include "macro.h"
/*----------------------------------------------------------------------------*/
/* Change PCONP register, reset value 0x042887DE */
inline void sysPowerEnable(enum sysPowerControl peripheral)
{
  LPC_SC->PCONP |= BIT(peripheral);
}
/*----------------------------------------------------------------------------*/
inline void sysPowerDisable(enum sysPowerControl peripheral)
{
  LPC_SC->PCONP &= ~BIT(peripheral);
}
/*----------------------------------------------------------------------------*/
///* Get peripheral clock divider for specified peripheral */
//inline enum sysClockDiv sysGetPeriphDiv(enum sysClockControl peripheral)
//{
//  uint32_t *ptr = (uint32_t *)(&LPC_SC->PCLKSEL0 + (peripheral >> 5));
//  return (*ptr >> (peripheral & 0x01F)) & 0x03;
//}
/*----------------------------------------------------------------------------*/
/* Exception for CAN1, CAN2 and CAN filtering: PCLK_DIV8 divides clock by 6 */
inline void sysSetPeriphDiv(enum sysClockControl peripheral,
    enum sysClockDiv divider)
{
  uint32_t *ptr = (uint32_t *)(&LPC_SC->PCLKSEL0 + (peripheral >> 5));
  *ptr = (*ptr & ~(3 << (peripheral & 0x01F))) |
      (divider << (peripheral & 0x01F));
}
/*----------------------------------------------------------------------------*/
