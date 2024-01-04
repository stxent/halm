/*
 * system.c
 * Copyright (C) 2024 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/platform/platform_defs.h>
#include <halm/platform/stm32/stm32f4xx/system_defs.h>
#include <halm/platform/stm32/system.h>
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
void sysClockDisable(enum SysClockBranch branch)
{
  const unsigned int index = branch >> 5;
  const uint32_t mask = ~(1UL << (branch & 0x1F));

  switch (index)
  {
    case INDEX_AHB1:
      STM_RCC->AHB1ENR &= mask;
      break;

    case INDEX_AHB2:
      STM_RCC->AHB2ENR &= mask;
      break;

    case INDEX_AHB3:
      STM_RCC->AHB3ENR &= mask;
      break;

    case INDEX_APB1:
      STM_RCC->APB1ENR &= mask;
      break;

    case INDEX_APB2:
      STM_RCC->APB2ENR &= mask;
      break;
  }
}
/*----------------------------------------------------------------------------*/
void sysClockEnable(enum SysClockBranch branch)
{
  const unsigned int index = branch >> 5;
  const uint32_t value = 1UL << (branch & 0x1F);

  switch (index)
  {
    case INDEX_AHB1:
      STM_RCC->AHB1ENR |= value;
      break;

    case INDEX_AHB2:
      STM_RCC->AHB2ENR |= value;
      break;

    case INDEX_AHB3:
      STM_RCC->AHB3ENR |= value;
      break;

    case INDEX_APB1:
      STM_RCC->APB1ENR |= value;
      break;

    case INDEX_APB2:
      STM_RCC->APB2ENR |= value;
      break;
  }
}
/*----------------------------------------------------------------------------*/
bool sysClockStatus(enum SysClockBranch branch)
{
  const unsigned int index = branch >> 5;
  const uint32_t mask = 1UL << (branch & 0x1F);
  uint32_t value = 0;

  switch (index)
  {
    case INDEX_AHB1:
      value = STM_RCC->AHB1ENR;
      break;

    case INDEX_AHB2:
      value = STM_RCC->AHB2ENR;
      break;

    case INDEX_AHB3:
      value = STM_RCC->AHB3ENR;
      break;

    case INDEX_APB1:
      value = STM_RCC->APB1ENR;
      break;

    case INDEX_APB2:
      value = STM_RCC->APB2ENR;
      break;
  }

  return (value & mask) != 0;
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
  uint32_t acr = STM_FLASH->ACR & ~(FLASH_ACR_LATENCY_MASK | FLASH_ACR_PRFTEN);

  /* Configure wait states */
  acr |= FLASH_ACR_LATENCY(value - 1);

  if (value > 1)
  {
    /* Enable prefetch buffer when the number of wait states is not zero */
    acr |= FLASH_ACR_PRFTEN;
    /* Enable instruction and data caches */
    acr |= FLASH_ACR_ICEN | FLASH_ACR_DCEN;
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
  const unsigned int index = block >> 5;
  const uint32_t value = 1UL << (block & 0x1F);

  switch (index)
  {
    case INDEX_AHB1:
      STM_RCC->AHB1RSTR |= value;
      break;

    case INDEX_AHB2:
      STM_RCC->AHB2RSTR |= value;
      break;

    case INDEX_AHB3:
      STM_RCC->AHB3RSTR |= value;
      break;

    case INDEX_APB1:
      STM_RCC->APB1RSTR |= value;
      break;

    case INDEX_APB2:
      STM_RCC->APB2RSTR |= value;
      break;
  }
}
/*----------------------------------------------------------------------------*/
void sysResetDisable(enum SysBlockReset block)
{
  const unsigned int index = block >> 5;
  const uint32_t mask = ~(1UL << (block & 0x1F));

  switch (index)
  {
    case INDEX_AHB1:
      STM_RCC->AHB1RSTR &= mask;
      break;

    case INDEX_AHB2:
      STM_RCC->AHB2RSTR &= mask;
      break;

    case INDEX_AHB3:
      STM_RCC->AHB3RSTR &= mask;
      break;

    case INDEX_APB1:
      STM_RCC->APB1RSTR &= mask;
      break;

    case INDEX_APB2:
      STM_RCC->APB2RSTR &= mask;
      break;
  }
}
