/*
 * system.c
 * Copyright (C) 2023 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/platform/numicro/m03x/system_defs.h>
#include <halm/platform/numicro/system.h>
/*----------------------------------------------------------------------------*/
static volatile uint32_t *calcClockReg(enum SysClockBranch);
static volatile uint32_t *calcResetReg(enum SysBlockReset);
static unsigned int calcFlashLatency(uint32_t);
static uint32_t getFlashSize(void);
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
  const uint32_t size = getFlashSize();

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
static uint32_t getFlashSize(void)
{
  const uint32_t pdid = NM_SYS->PDID;
  const uint32_t group = (pdid & 0xF00) >> 8;
  const uint32_t part = pdid & 0xFF;

  if (group == 0x1)
  {
    if (part == 0x10 || part == 0x40)
      return 512 * 1024;
    else
      return 0;
  }
  if (group == 0x6)
  {
    switch (part)
    {
      case 0xE1:
      case 0xE2:
      case 0xF1:
      case 0xF2:
        return 32 * 1024;

      case 0xE0:
      case 0xF0:
        return 64 * 1024;

      case 0x00:
      case 0x01:
      case 0x10:
      case 0x11:
      case 0x40:
      case 0x41:
        return 256 * 1024;

      default:
        return 0;
    }
  }
  else if (group == 0xB)
  {
    if (part == 0xA0 || part == 0xB0 || part == 0xE0)
      return 16 * 1024;
    else
      return 0;
  }
  else if (group == 0xC)
  {
    if (part == 0xA0 || part == 0xB0 || part == 0xE0)
      return 32 * 1024;
    else
      return 0;
  }
  else if (group == 0xD)
  {
    switch (group)
    {
      case 0x01:
      case 0x11:
        return 32 * 1024;

      case 0x00:
      case 0x10:
      case 0x90:
      case 0xB0:
      case 0xE0:
        return 64 * 1024;

      default:
        return 0;
    }
  }
  else if (group == 0xE)
  {
    if (part == 0xE0 || part == 0xE1 || part == 0xE9 || part == 0xEE)
      return 128 * 1024;
    else
      return 0;
  }
  else
    return 0;
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
