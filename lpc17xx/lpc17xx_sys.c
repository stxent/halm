/*
 * lpc17xx_sys.c
 *
 *  Created on: Oct 19, 2012
 *      Author: xen
 */

#include "lpc17xx_sys.h"
#include "LPC17xx.h"
/*----------------------------------------------------------------------------*/
/* Get peripheral clock divider for specified peripheral */
inline uint8_t sysGetPeriphDiv(enum sysPeriphClock device)
{
  uint32_t *ptr = (uint32_t *)(&LPC_SC->PCLKSEL0 + (device >> 5));
  return (*ptr >> (device & 0x01F)) & 0x03;
}
/*----------------------------------------------------------------------------*/
/*
 * 0: divide by 4
 * 1: divide by 1
 * 2: divide by 2
 * 3: divide by 8 (or 6 for CAN)
 */
inline void sysSetPeriphDiv(enum sysPeriphClock device, uint8_t divider)
{
  uint32_t *ptr = (uint32_t *)(&LPC_SC->PCLKSEL0 + (device >> 5));
  *ptr = (*ptr & ~(3 << (device & 0x01F))) | (divider << (device & 0x01F));
}
/*----------------------------------------------------------------------------*/
