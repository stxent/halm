/*
 * system.c
 * Copyright (C) 2016 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/platform/platform_defs.h>
#include <halm/platform/stm32/stm32f1xx/clocking_defs.h>
#include <halm/platform/stm32/stm32f1xx/system_defs.h>
#include <halm/platform/stm32/system.h>
#include <stddef.h>
/*----------------------------------------------------------------------------*/
enum
{
  INDEX_AHB,
  INDEX_APB1,
  INDEX_APB2
};
/*----------------------------------------------------------------------------*/
static inline volatile uint32_t *blockToResetReg(enum SysBlockReset);
static inline volatile uint32_t *branchToEnableReg(enum SysClockBranch);
/*----------------------------------------------------------------------------*/
static inline volatile uint32_t *blockToResetReg(enum SysBlockReset block)
{
  switch (block >> 5)
  {
    case INDEX_AHB:
      return &STM_RCC->AHBRSTR;

    case INDEX_APB1:
      return &STM_RCC->APB1RSTR;

    case INDEX_APB2:
      return &STM_RCC->APB2RSTR;

    default:
      return NULL;
  }
}
/*----------------------------------------------------------------------------*/
static inline volatile uint32_t *branchToEnableReg(enum SysClockBranch branch)
{
  switch (branch >> 5)
  {
    case INDEX_AHB:
      return &STM_RCC->AHBENR;

    case INDEX_APB1:
      return &STM_RCC->APB1ENR;

    case INDEX_APB2:
      return &STM_RCC->APB2ENR;

    default:
      return NULL;
  }
}
/*----------------------------------------------------------------------------*/
void sysClockDisable(enum SysClockBranch branch)
{
  volatile uint32_t * const reg = branchToEnableReg(branch);
  const uint32_t mask = 1UL << (branch & 0x1F);

  *reg &= ~mask;
}
/*----------------------------------------------------------------------------*/
void sysClockEnable(enum SysClockBranch branch)
{
  volatile uint32_t * const reg = branchToEnableReg(branch);
  const uint32_t mask = 1UL << (branch & 0x1F);

  *reg |= mask;
}
/*----------------------------------------------------------------------------*/
bool sysClockStatus(enum SysClockBranch branch)
{
  volatile uint32_t * const reg = branchToEnableReg(branch);
  const uint32_t mask = 1UL << (branch & 0x1F);

  return (*reg & mask) != 0;
}
/*----------------------------------------------------------------------------*/
unsigned int sysFlashLatency(void)
{
  return FLASH_ACR_LATENCY(STM_FLASH->ACR) + 1;
}
/*----------------------------------------------------------------------------*/
/**
 * Set the flash access time.
 * @param value Flash access time in CPU clocks.
 * @n Possible values and recommended operating frequencies:
 *   - 1 clock: up to 24 MHz.
 *   - 2 clocks: up to 48 MHz.
 *   - 3 clocks: up to 72 MHz.
 */
void sysFlashLatencyUpdate(unsigned int value)
{
  const uint32_t prescaler = CFGR_HPRE_VALUE(STM_RCC->CFGR);
  uint32_t acr = STM_FLASH->ACR & ~(FLASH_ACR_LATENCY_MASK | FLASH_ACR_PRFTBE);

  /* Configure wait states */
  acr |= FLASH_ACR_LATENCY(value - 1);

  if ((value > 1) || (prescaler & 0x08) != 0)
  {
    /*
     * Enable a prefetch buffer when the number of wait states is not zero
     * or the AHB prescaler is enabled.
     */
    acr |= FLASH_ACR_PRFTBE;
  }

  STM_FLASH->ACR = acr;
}
/*----------------------------------------------------------------------------*/
void sysResetEnable(enum SysBlockReset block)
{
  volatile uint32_t * const reg = blockToResetReg(block);
  const uint32_t mask = 1UL << (block & 0x1F);

  *reg |= mask;
}
/*----------------------------------------------------------------------------*/
void sysResetDisable(enum SysBlockReset block)
{
  volatile uint32_t * const reg = blockToResetReg(block);
  const uint32_t mask = 1UL << (block & 0x1F);

  *reg &= ~mask;
}
/*----------------------------------------------------------------------------*/
void sysResetPulse(enum SysBlockReset block)
{
  volatile uint32_t * const reg = blockToResetReg(block);
  const uint32_t mask = 1UL << (block & 0x1F);

  *reg |= mask;
  *reg &= ~mask;
}
