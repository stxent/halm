/*
 * lpc17xx_sys.c
 * Copyright (C) 2012 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <LPC17xx.h>
#include "system.h"
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
inline void usleep(uint32_t period)
{
  volatile uint32_t count = SystemCoreClock / 3000000 * period;

  __asm__ __volatile__ (
      "1: SUBS.W %[count], %[count], #1 \n"
      "BNE 1b \n"
      : [count] "=r"(count)
      : "0" (count)
      : "r3"
  );
}
/*----------------------------------------------------------------------------*/
inline void msleep(uint32_t period)
{
  usleep(1000 * period);
}
