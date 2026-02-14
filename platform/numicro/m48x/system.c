/*
 * system.c
 * Copyright (C) 2023 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/platform/numicro/flash_defs.h>
#include <halm/platform/numicro/system.h>
#include <halm/platform/numicro/system_defs.h>
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
  /* CYCLE value of 1 is not used */
  if (frequency < 27000000)
    return 0;
  else if (frequency <= 54000000)
    return 2;
  else if (frequency <= 81000000)
    return 3;
  else if (frequency <= 108000000)
    return 4;
  else if (frequency <= 135000000)
    return 5;
  else if (frequency <= 162000000)
    return 6;
  else if (frequency <= 192000000)
    return 7;
  else
    return 8;
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
void sysConfigVoltageReference(void)
{
  uint32_t vrefctl = NM_SYS->VREFCTL & ~VREFCTL_VREFCTL_MASK;

#ifndef CONFIG_PLATFORM_NUMICRO_VREF_EXT
  /* Recommended by manual */
  vrefctl = (vrefctl & ~VREFCTL_VBGISEL_MASK) | VREFCTL_VBGISEL(VBGISEL_10UA4);
#endif

#ifdef CONFIG_PLATFORM_NUMICRO_VREF_1V6
  vrefctl |= VREFCTL_VREFCTL(VREFCTL_INT_1V6);
#elifdef CONFIG_PLATFORM_NUMICRO_VREF_2V0
  vrefctl |= VREFCTL_VREFCTL(VREFCTL_INT_2V0);
#elifdef CONFIG_PLATFORM_NUMICRO_VREF_2V5
  vrefctl |= VREFCTL_VREFCTL(VREFCTL_INT_2V5);
#elifdef CONFIG_PLATFORM_NUMICRO_VREF_3V0
  vrefctl |= VREFCTL_VREFCTL(VREFCTL_INT_3V0);
#else
  vrefctl |= VREFCTL_VREFCTL(VREFCTL_EXT);
#endif

  sysUnlockReg();
  NM_SYS->VREFCTL = vrefctl;
  sysLockReg();
}
/*----------------------------------------------------------------------------*/
void sysFlashLatencyReset(void)
{
  sysUnlockReg();
  NM_FMC->CYCCTL = (NM_FMC->CYCCTL & ~CYCCTL_CYCLE_MASK)
      | CYCCTL_CYCLE(CYCCTL_CYCLE_MAX);
  sysLockReg();
}
/*----------------------------------------------------------------------------*/
void sysFlashLatencyUpdate(uint32_t frequency)
{
  const unsigned int latency = calcFlashLatency(frequency);

  sysUnlockReg();
  NM_FMC->CYCCTL = (NM_FMC->CYCCTL & ~CYCCTL_CYCLE_MASK)
      | CYCCTL_CYCLE(latency);
  sysLockReg();
}
/*----------------------------------------------------------------------------*/
size_t sysGetSizeAPROM(void)
{
  const uint32_t pdid = NM_SYS->PDID;
  const uint32_t group = (pdid & 0xFF00000) >> 20;
  const uint32_t part = pdid & 0xFF;

  if (group == 0x13)
  {
    switch (part)
    {
      case 0x02:
      case 0x12:
      case 0xE2:
        return 128 * 1024;

      case 0x00:
      case 0x10:
      case 0x14:
      case 0x40:
      case 0x44:
      case 0xE0:
        return 256 * 1024;

      default:
        return 0;
    }
  }
  else if (group == 0x0D)
  {
    return 512 * 1024;
  }
  else
    return 0;
}
/*----------------------------------------------------------------------------*/
void sysPowerLevelReset(void)
{
  sysUnlockReg();
  NM_SYS->PLCTL = (NM_SYS->PLCTL & ~PLCTL_PLSEL_MASK) | PLCTL_PLSEL(PLSEL_PL0);
  sysLockReg();

  /* Wait until core voltage change is completed */
  while (NM_SYS->PLSTS & PLSTS_PLCBUSY);
}
/*----------------------------------------------------------------------------*/
void sysPowerLevelUpdate(uint32_t frequency)
{
  uint32_t value = NM_SYS->PLCTL & ~PLCTL_PLSEL_MASK;

  if (frequency <= 160000000)
    value |= PLCTL_PLSEL(PLSEL_PL1);
  else
    value |= PLCTL_PLSEL(PLSEL_PL0);

  sysUnlockReg();
  NM_SYS->PLCTL = value;
  sysLockReg();

  /* Wait until core voltage change is completed */
  while (NM_SYS->PLSTS & PLSTS_PLCBUSY);
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
