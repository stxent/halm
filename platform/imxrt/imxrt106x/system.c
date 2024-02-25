/*
 * system.c
 * Copyright (C) 2024 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/platform/imxrt/imxrt106x/system_defs.h>
#include <halm/platform/imxrt/system.h>
#include <halm/platform/platform_defs.h>
/*----------------------------------------------------------------------------*/
void sysClockDisable(enum SysClockBranch branch)
{
  const uint32_t index = branch >> 4;
  const uint32_t offset = branch & 0xF;

  IMX_CCM->CCGR[index] = (IMX_CCM->CCGR[index] & ~CCGR_CG_MASK(offset))
      | CCGR_CG(CG_OFF, offset);
}
/*----------------------------------------------------------------------------*/
void sysClockEnable(enum SysClockBranch branch)
{
  const uint32_t index = branch >> 4;
  const uint32_t offset = branch & 0xF;

  IMX_CCM->CCGR[index] = (IMX_CCM->CCGR[index] & ~CCGR_CG_MASK(offset))
      | CCGR_CG(CG_ON_ALL_MODES, offset);
}
/*----------------------------------------------------------------------------*/
bool sysClockStatus(enum SysClockBranch branch)
{
  const uint32_t index = branch >> 4;
  const uint32_t offset = branch & 0xF;

  return CCGR_CG_VALUE(IMX_CCM->CCGR[index], offset) == CG_ON_ALL_MODES;
}
