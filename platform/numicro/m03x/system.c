/*
 * system.c
 * Copyright (C) 2023 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/platform/numicro/flash_defs.h>
#include <halm/platform/numicro/m03x/system_defs.h>
#include <halm/platform/numicro/system.h>
/*----------------------------------------------------------------------------*/
static volatile uint32_t *calcClockReg(enum SysClockBranch);
static volatile uint32_t *calcResetReg(enum SysBlockReset);
static unsigned int calcFlashLatency(uint32_t);
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
static unsigned int calcFlashLatency(uint32_t frequency)
{
  const size_t size = sysGetSizeAPROM();

  if (!size)
  {
    /* Unknown part, use safe settings */
    return 0;
  }
  if (size <= 128 * 1024)
  {
    /* Parts with 16/32/64/128 KiB Flash */
    return frequency <= 24000000 ? 1 : 0;
  }
  else
  {
    /* Parts with 256/512 KiB Flash */
    if (frequency <= 12000000)
      return 1;
    else if (frequency <= 36000000)
      return 2;
    else if (frequency <= 60000000)
      return 3;
    else
      return 0;
  }
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
size_t sysGetSizeAPROM(void)
{
  const uint32_t pdid = NM_SYS->PDID;
  const uint32_t group = pdid & 0xFFF;

  if ((group & 0xF00) == 0x100)
  {
    return 512 * 1024;
  }
  else if ((group & 0xF00) == 0x600)
  {
    const uint8_t high = (group & 0x0F0) >> 4;

    if (high == 0xE || high == 0xF)
    {
      return (group & 0x00F) ? 32 * 1024 : 64 * 1024;
    }
    else if (high == 0x0 || high == 0x1 || high == 0x4)
    {
      return 256 * 1024;
    }
    else
      return 0;
  }
  else if ((group & 0xF00) == 0xB00)
  {
    return 16 * 1024;
  }
  else if ((group & 0xF00) == 0xC00)
  {
    return 32 * 1024;
  }
  else if ((group & 0xF00) == 0xD00)
  {
    return (group & 0x00F) ? 32 * 1024 : 64 * 1024;
  }
  else if ((group & 0xF00) == 0xE00)
  {
    return 128 * 1024;
  }
  else
    return 0;
}
/*----------------------------------------------------------------------------*/
void sysFlashLatencyReset(void)
{
  sysUnlockReg();
  NM_FMC->FTCTL = (NM_FMC->FTCTL & ~FTCTL_FOM_MASK) | FTCTL_FOM(0);
  sysLockReg();
}
/*----------------------------------------------------------------------------*/
void sysFlashLatencyUpdate(uint32_t frequency)
{
  const unsigned int latency = calcFlashLatency(frequency);

  sysUnlockReg();
  NM_FMC->FTCTL = (NM_FMC->FTCTL & ~FTCTL_FOM_MASK) | FTCTL_FOM(latency);
  sysLockReg();
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
    NM_SYS->REGLCTL = REGLCTL_MAGIC_NUMBER_0;
    NM_SYS->REGLCTL = REGLCTL_MAGIC_NUMBER_1;
    NM_SYS->REGLCTL = REGLCTL_MAGIC_NUMBER_2;
  }
  while (NM_SYS->REGLCTL == 0);
}
