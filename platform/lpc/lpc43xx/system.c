/*
 * system.c
 * Copyright (C) 2012 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/platform/lpc/lpc43xx/system_defs.h>
#include <halm/platform/lpc/system.h>
/*----------------------------------------------------------------------------*/
static LPC_CCU_BRANCH_Type *calcBranchReg(enum SysClockBranch);
/*----------------------------------------------------------------------------*/
static LPC_CCU_BRANCH_Type *calcBranchReg(enum SysClockBranch branch)
{
  if (branch < 0x200)
    return &LPC_CCU1->BRANCH[branch];
  else
    return &LPC_CCU2->BRANCH[branch - 0x200];
}
/*----------------------------------------------------------------------------*/
void sysClockEnable(enum SysClockBranch branch)
{
  calcBranchReg(branch)->CFG |= CFG_RUN | CFG_AUTO | CFG_WAKEUP;
}
/*----------------------------------------------------------------------------*/
void sysClockDisable(enum SysClockBranch branch)
{
  LPC_CCU_BRANCH_Type * const reg = calcBranchReg(branch);

  /* Use AHB disable protocol and do not enable clock after wake up */
  reg->CFG = (reg->CFG & ~CFG_WAKEUP) | CFG_AUTO;
  reg->CFG &= ~CFG_RUN; /* Disable clock */
}
/*----------------------------------------------------------------------------*/
bool sysClockStatus(enum SysClockBranch branch)
{
  return (calcBranchReg(branch)->STAT & STAT_RUN) != 0;
}
/*----------------------------------------------------------------------------*/
void sysCoreM0AppRemap(uintptr_t address)
{
  LPC_CREG->M0APPMEMMAP = address;
}
/*----------------------------------------------------------------------------*/
void sysCoreM0SubRemap(uintptr_t address)
{
  LPC_CREG->M0SUBMEMMAP = address;
}
/*----------------------------------------------------------------------------*/
void sysFlashEnable(unsigned int bank)
{
  /* Flash bank A or B */
  LPC_CREG->FLASHCFG[bank] |= FLASHCFG_POW;
}
/*----------------------------------------------------------------------------*/
void sysFlashDisable(unsigned int bank)
{
  LPC_CREG->FLASHCFG[bank] &= ~FLASHCFG_POW;
}
/*----------------------------------------------------------------------------*/
unsigned int sysFlashLatency(void)
{
  /* Return access time for one of the banks */
  return FLASHCFG_FLASHTIM_VALUE(LPC_CREG->FLASHCFGA) + 1;
}
/*----------------------------------------------------------------------------*/
/**
 * Set the flash access time.
 * @param value Flash access time in CPU clocks.
 * @n Possible values and recommended operating frequencies:
 *   - 1 clock: up to 21 MHz.
 *   - 2 clocks: up to 43 MHz.
 *   - 3 clocks: up to 64 MHz.
 *   - 4 clocks: up to 86 MHz.
 *   - 5 clocks: up to 107 MHz.
 *   - 6 clocks: up to 129 MHz.
 *   - 7 clocks: up to 150 MHz.
 *   - 8 clocks: up to 172 MHz.
 *   - 9 clocks: up to 193 MHz.
 *   - 10 clocks: up to 204 MHz, safe setting for all allowed conditions.
 */
void sysFlashLatencyUpdate(unsigned int value)
{
  const uint32_t data = FLASHCFG_FLASHTIM(value - 1);

  /* Update flash access time for all flash banks */
  LPC_CREG->FLASHCFGA = (LPC_CREG->FLASHCFGA & ~FLASHCFG_FLASHTIM_MASK) | data;
  LPC_CREG->FLASHCFGB = (LPC_CREG->FLASHCFGB & ~FLASHCFG_FLASHTIM_MASK) | data;
}
/*----------------------------------------------------------------------------*/
void sysFlashLatencyReset(void)
{
  sysFlashLatencyUpdate(10);
}
/*----------------------------------------------------------------------------*/
void sysResetEnable(enum SysBlockReset block)
{
  const uint32_t mask = 1UL << (block & 0x1F);
  const unsigned int index = block >> 5;

  LPC_RGU->RESET_CTRL[index] = ~LPC_RGU->RESET_ACTIVE_STATUS[index] | mask;

  if (block != RST_M0SUB && block != RST_M0APP)
  {
    /* Wait for reset to be cleared */
    while (!(LPC_RGU->RESET_ACTIVE_STATUS[index] & mask));
  }
}
/*----------------------------------------------------------------------------*/
void sysResetDisable(enum SysBlockReset block)
{
  const uint32_t mask = 1UL << (block & 0x1F);
  const unsigned int index = block >> 5;

  if (block == RST_M0SUB || block == RST_M0APP)
    LPC_RGU->RESET_CTRL[index] = ~(LPC_RGU->RESET_ACTIVE_STATUS[index] | mask);
}
