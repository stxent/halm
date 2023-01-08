/*
 * system.c
 * Copyright (C) 2023 xent
 * Project is distributed under the terms of the MIT License
 */

// #include <halm/platform/numicro/m03x/system_defs.h> // TODO
#include <halm/platform/numicro/system.h>
/*----------------------------------------------------------------------------*/
static volatile uint32_t *calcClockReg(enum SysClockBranch);
static volatile uint32_t *calcResetReg(enum SysBlockReset);
static inline bool isClockProtected(enum SysClockBranch);
static inline bool isResetProtected(enum SysBlockReset);
/*----------------------------------------------------------------------------*/
static volatile uint32_t *calcClockReg(enum SysClockBranch branch)
{
  if (branch < 0x20)
    return &NM_CLK->AHBCLK;
  else if (branch < 0x40)
    return &NM_CLK->APBCLK0;
  else
    return &NM_CLK->APBCLK1;
}
/*----------------------------------------------------------------------------*/
static volatile uint32_t *calcResetReg(enum SysBlockReset block)
{
  if (block < 0x20)
    return &NM_SYS->IPRST0;
  else if (block < 0x40)
    return &NM_SYS->IPRST1;
  else
    return &NM_SYS->IPRST2;
}
/*----------------------------------------------------------------------------*/
static inline bool isClockProtected(enum SysClockBranch branch)
{
  return branch == CLK_WDT;
}
/*----------------------------------------------------------------------------*/
static inline bool isResetProtected(enum SysBlockReset block)
{
  return block <= RST_CRC;
}
/*----------------------------------------------------------------------------*/
void sysClockDisable(enum SysClockBranch branch)
{
  const bool protected = isClockProtected(branch);
  const uint32_t value = 1UL << (branch & 0x1F);

  if (protected)
    sysUnlockReg();

  *calcClockReg(branch) &= ~value;

  if (protected)
    sysLockReg();
}
/*----------------------------------------------------------------------------*/
void sysClockEnable(enum SysClockBranch branch)
{
  const bool protected = isClockProtected(branch);
  const uint32_t value = 1UL << (branch & 0x1F);

  if (protected)
    sysUnlockReg();

  *calcClockReg(branch) |= value;

  if (protected)
    sysLockReg();
}
/*----------------------------------------------------------------------------*/
bool sysClockStatus(enum SysClockBranch branch)
{
  return (*calcClockReg(branch) & (1UL << (branch & 0x1F))) != 0;
}
/*----------------------------------------------------------------------------*/
void sysResetBlock(enum SysBlockReset block)
{
  const bool protected = isResetProtected(block);
  const uint32_t value = 1UL << (block & 0x1F);
  volatile uint32_t * const reg = calcResetReg(block);

  if (protected)
    sysUnlockReg();

  *reg |= value;
  *reg &= ~value;

  if (protected)
    sysLockReg();
}
/*----------------------------------------------------------------------------*/
void sysLockReg(void)
{
  NM_SYS->REGLCTL = 0;
}
/*----------------------------------------------------------------------------*/
void sysUnlockReg(void)
{
  do
  {
    NM_SYS->REGLCTL = 0x59;
    NM_SYS->REGLCTL = 0x16;
    NM_SYS->REGLCTL = 0x88;
  }
  while (NM_SYS->REGLCTL == 0);
}
