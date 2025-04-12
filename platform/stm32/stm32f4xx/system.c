/*
 * system.c
 * Copyright (C) 2024 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/platform/platform_defs.h>
#include <halm/platform/stm32/stm32f4xx/system_defs.h>
#include <halm/platform/stm32/system.h>
#include <stddef.h>
/*----------------------------------------------------------------------------*/
enum
{
  INDEX_AHB1,
  INDEX_AHB2,
  INDEX_AHB3,
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
    case INDEX_AHB1:
      return &STM_RCC->AHB1RSTR;

    case INDEX_AHB2:
      return &STM_RCC->AHB2RSTR;

    case INDEX_AHB3:
      return &STM_RCC->AHB3RSTR;

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
    case INDEX_AHB1:
      return &STM_RCC->AHB1ENR;

    case INDEX_AHB2:
      return &STM_RCC->AHB2ENR;

    case INDEX_AHB3:
      return &STM_RCC->AHB3ENR;

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
 * @n Possible values and operating frequencies for 2.7V - 3.6V voltage range:
 *   - 1 clock: up to 30 MHz.
 *   - 2 clocks: up to 60 MHz.
 *   - 3 clocks: up to 90 MHz.
 *   - 4 clocks: up to 120 MHz.
 *   - 5 clocks: up to 150 MHz.
 *   - 6 clocks: up to 168 MHz.
 */
void sysFlashLatencyUpdate(unsigned int value)
{
  /* Configure wait states */
  uint32_t acr = FLASH_ACR_LATENCY(value - 1);

  if (value > 1)
  {
    /* Disable instruction and data caches before cache clearing */
    STM_FLASH->ACR &= ~(FLASH_ACR_ICEN | FLASH_ACR_DCEN);
    /* Clear instruction and data caches */
    STM_FLASH->ACR |= FLASH_ACR_ICRST | FLASH_ACR_DCRST;

    /* Enable prefetch buffer, instruction and data caches */
    acr |= FLASH_ACR_PRFTEN | FLASH_ACR_ICEN | FLASH_ACR_DCEN;
  }

  STM_FLASH->ACR = acr;
}
/*----------------------------------------------------------------------------*/
void sysPowerScalingUpdate(bool state)
{
  if (state)
  {
    /* Enable high-power mode */
    STM_PWR->CR |= PWR_CR_VOS;
  }
  else
    STM_PWR->CR &= ~PWR_CR_VOS;
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
