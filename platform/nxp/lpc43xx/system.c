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

  *reg |= CFG_RUN | CFG_AUTO | CFG_WAKEUP;
}
/*----------------------------------------------------------------------------*/
void sysClockDisable(enum sysClockBranch branch)
{
  volatile uint32_t *reg = calcBranchReg(branch);

  /* Use AHB disable protocol and do not enable clock after wake up */
  *reg = (*reg & ~CFG_WAKEUP) | CFG_AUTO;
  *reg &= ~CFG_RUN; /* Disable clock */
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
unsigned int sysFlashLatency()
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
void sysResetEnable(enum sysBlockReset block)
{
  const uint32_t mask = BIT(block & 0x1F);
  const unsigned int index = block >> 5;

  LPC_RGU->RESET_CTRL[index] = mask;

  if (block != RST_M0SUB && block != RST_M0APP)
    while (!(LPC_RGU->RESET_ACTIVE_STATUS[index] & mask));
}
/*----------------------------------------------------------------------------*/
void sysResetDisable(enum sysBlockReset block)
{
  if (block == RST_M0SUB || block == RST_M0APP)
    LPC_RGU->RESET_CTRL[block >> 5] = 0;
}
