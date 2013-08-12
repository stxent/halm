/*
 * power.c
 * Copyright (C) 2013 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include "macro.h"
#include "platform/nxp/device_defs.h"
#include "platform/nxp/lpc13xx/power.h"
/*----------------------------------------------------------------------------*/
/* Changes SYSAHBCLKCTRL register, reset value 0x0000485F */
void sysClockEnable(enum sysClockDevice peripheral)
{
  LPC_SYSCON->SYSAHBCLKCTRL |= BIT(peripheral);
}
/*----------------------------------------------------------------------------*/
void sysClockDisable(enum sysClockDevice peripheral)
{
  LPC_SYSCON->SYSAHBCLKCTRL &= ~BIT(peripheral);
}
