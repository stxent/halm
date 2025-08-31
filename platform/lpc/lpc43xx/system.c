/*
 * system.c
 * Copyright (C) 2012 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/platform/lpc/flash_defs.h>
#include <halm/platform/lpc/lpc43xx/system_defs.h>
#include <halm/platform/lpc/system.h>
#include <halm/platform/platform_defs.h>
#include <xcore/asm.h>
#include <assert.h>
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
void sysClockSetDivider(enum SysClockBranch branch, uint32_t value)
{
  assert(branch == CLK_M4_EMCDIV);
  assert(value >= 1 && value <= 2);

  LPC_CCU_BRANCH_Type * const reg = calcBranchReg(branch);

  reg->CFG = (reg->CFG & ~CFG_DIV_MASK) | CFG_DIV(value - 1);
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
  __dsb();
  __isb();
}
/*----------------------------------------------------------------------------*/
void sysCoreM0SubRemap(uintptr_t address)
{
  LPC_CREG->M0SUBMEMMAP = address;
  __dsb();
  __isb();
}
/*----------------------------------------------------------------------------*/
void sysCoreM4Remap(uintptr_t address)
{
  LPC_CREG->M4MEMMAP = address;
  __dsb();
  __isb();
}
/*----------------------------------------------------------------------------*/
bool sysFlashAvailable(void)
{
  const uint32_t id = LPC_CREG->CHIPID;
  return id == CHIPID_LPC43XX_REV_A || id == CHIPID_LPC43XX_REV_DASH;
}
/*----------------------------------------------------------------------------*/
void sysFlashDisable(unsigned int bank)
{
  /* Flash bank A or B */
  LPC_CREG->FLASHCFG[bank] &= ~FLASHCFG_POW;
}
/*----------------------------------------------------------------------------*/
void sysFlashEnable(unsigned int bank)
{
  /* Flash bank A or B */
  LPC_CREG->FLASHCFG[bank] |= FLASHCFG_POW;
}
/*----------------------------------------------------------------------------*/
unsigned int sysFlashLatency(void)
{
  if (!sysFlashAvailable())
    return 0;

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
  if (FLASHCFG_INT_CONTROL_VALUE(LPC_CREG->FLASHCFGA) != 0)
  {
    LPC_CREG->FLASHCFGA = (LPC_CREG->FLASHCFGA & ~FLASHCFG_FLASHTIM_MASK)
        | FLASHCFG_FLASHTIM(value - 1);
  }

  if (FLASHCFG_INT_CONTROL_VALUE(LPC_CREG->FLASHCFGB) != 0)
  {
    LPC_CREG->FLASHCFGB = (LPC_CREG->FLASHCFGB & ~FLASHCFG_FLASHTIM_MASK)
        | FLASHCFG_FLASHTIM(value - 1);
  }
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
