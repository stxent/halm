/*
 * system.c
 * Copyright (C) 2023 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/platform/numicro/flash_defs.h>
#include <halm/platform/numicro/m03x/system_defs.h>
#include <halm/platform/numicro/system.h>
#include <assert.h>
/*----------------------------------------------------------------------------*/
static volatile uint32_t *calcClockReg(enum SysClockBranch);
static volatile uint32_t *calcResetReg(enum SysBlockReset);
static bool calcFlashLatency(uint32_t, unsigned int *, bool *);
static inline bool isClockProtected(enum SysClockBranch);
static inline bool isResetProtected(enum SysBlockReset);
static void setFlashLatency(unsigned int);
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
static bool calcFlashLatency(uint32_t frequency, unsigned int *cycles,
    bool *cache)
{
  const size_t size = sysGetSizeAPROM();
  assert(size);

  if (size <= 128 * 1024)
  {
    *cache = false;

    /* Parts with 16/32/64/128 KiB Flash */
    if (frequency <= 24000000)
    {
      *cycles = 1;
      return false;
    }
    else
    {
      *cycles = 2;
      return true;
    }
  }
  else
  {
    *cache = true;

    /* Parts with 256/512 KiB Flash */
    if (frequency <= 12000000)
    {
      *cycles = 1;
      return false;
    }
    else if (frequency <= 36000000)
    {
      *cycles = 2;
      return false;
    }
    else if (frequency <= 60000000)
    {
      *cycles = 3;
      return false;
    }
    else
    {
      *cycles = 4;
      return true;
    }
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
static void setFlashLatency(unsigned int value)
{
  sysUnlockReg();
  NM_FMC->FTCTL = (NM_FMC->FTCTL & ~FTCTL_FOM_MASK) | FTCTL_FOM(value);
  sysLockReg();
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
  setFlashLatency(0);
}
/*----------------------------------------------------------------------------*/
unsigned int sysFlashLatencyUpdate(uint32_t frequency)
{
  unsigned int cycles;
  bool cache;

  const bool safeLatencyValue = calcFlashLatency(frequency, &cycles, &cache);

  setFlashLatency(safeLatencyValue ? 0 : cycles);
  return cache ? 0 : cycles;
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
