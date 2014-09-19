/*
 * system.c
 * Copyright (C) 2012 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <platform/nxp/lpc43xx/system.h>
#include <platform/nxp/lpc43xx/system_defs.h>
/*----------------------------------------------------------------------------*/
void sysResetEnable(enum sysDeviceReset block)
{
  const uint32_t mask = BIT(block & 0x1F);
  const uint8_t index = block >> 5;

  LPC_RGU->RESET_CTRL[index] = mask;

  if (block != RST_M0SUB && block != RST_M0APP)
    while (!(LPC_RGU->RESET_ACTIVE_STATUS[index] & mask));
}
/*----------------------------------------------------------------------------*/
void sysResetDisable(enum sysDeviceReset block)
{
  if (block == RST_M0SUB || block == RST_M0APP)
    LPC_RGU->RESET_CTRL[block >> 5] = 0;
}
