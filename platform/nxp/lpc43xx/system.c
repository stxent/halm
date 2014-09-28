/*
 * system.c
 * Copyright (C) 2012 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <platform/nxp/lpc43xx/system.h>
#include <platform/nxp/lpc43xx/system_defs.h>
/*----------------------------------------------------------------------------*/
static volatile uint32_t *calcBranchReg(enum sysClockBranch branch)
{
  const uint32_t address = branch < 0x200 ? (uint32_t)LPC_CCU1 + (branch << 3)
      : (uint32_t)LPC_CCU2 + ((branch - 0x200) << 3);

  return (volatile uint32_t *)address;
}
/*----------------------------------------------------------------------------*/
void sysClockEnable(enum sysClockBranch branch)
{
  volatile uint32_t *reg = calcBranchReg(branch);

  *reg = (*reg & ~CFG_AUTO) | CFG_RUN;
}
/*----------------------------------------------------------------------------*/
void sysClockDisable(enum sysClockBranch branch)
{
  volatile uint32_t *reg = calcBranchReg(branch);

  *reg |= CFG_AUTO; /* Initiate clock disable process */
  *reg &= ~CFG_RUN; /* Disable clock */
  /* TODO Check whether bit clearing necessary */
  *reg &= ~CFG_AUTO;
}
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
