/*
 * lpc13xx_sys.c
 * Copyright (C) 2013 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include "LPC13xx.h"
/*----------------------------------------------------------------------------*/
#include "macro.h"
#include "lpc13xx_sys.h"
/*----------------------------------------------------------------------------*/
/* Changes SYSAHBCLKCTRL register, reset value 0x0000485F */
inline void sysClockEnable(enum sysClockDevice peripheral)
{
  LPC_SYSCON->SYSAHBCLKCTRL |= BIT(peripheral);
}
/*----------------------------------------------------------------------------*/
inline void sysClockDisable(enum sysClockDevice peripheral)
{
  LPC_SYSCON->SYSAHBCLKCTRL &= ~BIT(peripheral);
}
